#include <iostream>
#include <map>
#include <thread>
#include <chrono>
#include "pixelreader/PixelReader.h"
#include "solver/GeneratorSolver.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx10.h"


struct GridInfo {
	int rows;
	int cols;
};


HWND getRobloxHWND() {
	HWND robloxHWND = FindWindowA(NULL, "Roblox");
	return robloxHWND;
}


bool getGridClientPosition(RECT &outGridRect) {
	HWND robloxHWND = getRobloxHWND();
	RECT robloxClientRect;
	bool success = GetClientRect(robloxHWND, &robloxClientRect);
	if (!success)
		return false;

	const int clientWidth = robloxClientRect.right - robloxClientRect.left;
	const int clientHeight = robloxClientRect.bottom - robloxClientRect.top;

	POINT leftTop = { robloxClientRect.left,  robloxClientRect.top };
	POINT rightBottom = { robloxClientRect.right,  robloxClientRect.bottom };
	success = ClientToScreen(robloxHWND, &leftTop) && ClientToScreen(robloxHWND, &rightBottom);
	if (!success)
		return false;
	RECT robloxScreenRect = { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };



	const double mainContainerAspectRatio = 0.86;

	double mainContainerWidth = clientWidth * 0.4;
	double mainContainerHeight = mainContainerWidth / mainContainerAspectRatio;
	if (mainContainerHeight > clientHeight) {
		mainContainerHeight = clientHeight;
		mainContainerWidth = mainContainerWidth * mainContainerAspectRatio;
	}

	double secondaryContainerWidth = mainContainerHeight * 0.625;
	double secondaryContainerHeight = mainContainerHeight * 0.625;

	int gridWidth = std::round(secondaryContainerWidth - 40);
	int gridHeight = std::round(secondaryContainerHeight - 40);

	

	outGridRect.left = std::round((clientWidth - gridWidth) / 2 + robloxScreenRect.left);
	outGridRect.top = std::round((clientHeight - gridHeight) / 2 + robloxScreenRect.top);
	outGridRect.right = outGridRect.left + gridWidth;
	outGridRect.bottom = outGridRect.top + gridHeight;

	printf_s("Grid X: %d, Grid Y: %d Grid Width: %d, Grid Height: %d\n", outGridRect.left, outGridRect.top, gridWidth, gridHeight);
	return true;
}

bool getGridCellDimensions(PixelReader& pixelReader, RECT inGridRect, GridInfo &outGridInfo) {
	int gridWidth = inGridRect.right - inGridRect.left;
	int gridHeight = inGridRect.bottom - inGridRect.top;

	const int cellOutlineColor = 0x141414;
	
	int lastColor = cellOutlineColor;
	int occurences = 0;

	bool success = pixelReader.CaptureScreenRegion(inGridRect.left, inGridRect.top, gridWidth, gridHeight);
	if (!success)
		return false;

	int pixelY = 5;
	for (int pixelX = 1; pixelX < gridWidth; pixelX++) {
		int pixelColor = pixelReader.PixelGetColor(pixelX, 5);
		if (pixelColor == cellOutlineColor && pixelColor != lastColor) {
			occurences += 1;
		}
		lastColor = pixelColor;
	}
	if (occurences < 2) {
		// Only found the outer borders or less
		std::cout << "SUPER ERROR!" << std::endl;
		return false;
	}
	outGridInfo.rows = occurences;
	outGridInfo.cols = occurences;
	
	printf_s("Rows: %d, Cols: %d\n", outGridInfo.rows, outGridInfo.cols);
	return true;
}
bool getNodePairs(PixelReader& pixelReader, RECT inGridRect, GridInfo inGridInfo, std::vector<NodePair> &outNodePairs) {
	int gridWidth = inGridRect.right - inGridRect.left;
	int gridHeight = inGridRect.bottom - inGridRect.top;

	int nodeReadPixel = std::round(0.20 * (1.f / inGridInfo.cols) * gridHeight);
	std::map<int, Cell> singleNodes;
	for (int cellRow = 0; cellRow < inGridInfo.rows; cellRow++) {
		for (int cellCol = 0; cellCol < inGridInfo.cols; cellCol++) {
			int cellX = std::round(((double)(cellCol + 0.5) / inGridInfo.cols) * gridWidth);
			int cellY = std::round(((double)(cellRow + 0.5) / inGridInfo.rows) * gridHeight);
			
			int pixelColor = pixelReader.PixelGetColor(cellX, cellY - nodeReadPixel);
			printf_s("Row: %d, Col: %d has color, 0x%X at (%d, %d)\n", cellRow, cellCol, pixelColor, cellX + inGridRect.left, cellY + inGridRect.top);
			if (pixelColor == 0x0E0E0E)
				continue;

			Cell thisNode = { cellRow, cellCol };

			if (singleNodes.find(pixelColor) == singleNodes.end()) {
				singleNodes[pixelColor] = thisNode;
				continue;
			}
				
			Cell otherNode = singleNodes[pixelColor];
			int pairIndex = outNodePairs.size();
			NodePair nodePair = { pairIndex, 65 + pairIndex, thisNode, otherNode };
			outNodePairs.push_back(nodePair);
		}
	}
	for (int row = 0; row < inGridInfo.rows; row++) {
		for (int col = 0; col < inGridInfo.cols; col++) {
			char space = '.';
			for (NodePair nodePair : outNodePairs) {
				if ((nodePair.firstNode.row == row && nodePair.firstNode.col == col) ||
					(nodePair.secondNode.row == row && nodePair.secondNode.col == col)
					)
					space = nodePair.contents;

			}
			std::string charString = std::string(1, space);
			std::cout << charString << " ";
		}
		std::cout << std::endl;
	}
	if (outNodePairs.size() == 0)
		return false;
	std::cout << outNodePairs.size() << std::endl;
	
	return true;
}
void MouseMove(int mouseX, int mouseY) {
	// Move the mouse to the specified coordinates
	SetCursorPos(mouseX, mouseY);
	mouse_event(MOUSEEVENTF_MOVE, 1, 0, 0, 0);

}
void performStuff(CellPath inCellPath, RECT inGridRect, GridInfo inGridInfo) {
	int gridWidth = inGridRect.right - inGridRect.left;
	int gridHeight = inGridRect.bottom - inGridRect.top;
	Cell firstCell = inCellPath.path[0];
	int cellX = std::round(((double)(firstCell.col + 0.5) / inGridInfo.cols) * gridWidth) + inGridRect.left;
	int cellY = std::round(((double)(firstCell.row + 0.5) / inGridInfo.rows) * gridHeight) + inGridRect.top;
	MouseMove(cellX, cellY);
	mouse_event(MOUSEEVENTF_LEFTDOWN, 1, 0, 0, 0);
	for (Cell cell : inCellPath.path) {
		if (cell == inCellPath.nodePair.firstNode)
			continue;
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
		int cellX = std::round(((double)(cell.col + 0.5) / inGridInfo.cols) * gridWidth) + inGridRect.left;
		int cellY = std::round(((double)(cell.row + 0.5) / inGridInfo.rows) * gridHeight) + inGridRect.top;
		MouseMove(cellX, cellY);
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	mouse_event(MOUSEEVENTF_LEFTUP, 1, 0, 0, 0);
}
void performThings(CellPath inCellPath, RECT inGridRect, GridInfo inGridInfo) {
	const int waitTime = 16;
	int gridWidth = inGridRect.right - inGridRect.left;
	int gridHeight = inGridRect.bottom - inGridRect.top;

	// Variables to track direction for skipping cells in a straight line
	int currentDeltaRow = 0;
	int currentDeltaCol = 0;
	int streak = 0;
	// Loop from the second cell to the end
	for (size_t i = 0; i < inCellPath.path.size() - 1; ++i) {
		Cell currentCell = inCellPath.path[i];
		Cell nextCell = inCellPath.path[i + 1];

		// Calculate the direction delta
		int nextDeltaRow = nextCell.row - currentCell.row;
		int nextDeltaCol = nextCell.col - currentCell.col;

		// Check if the direction has changed from the last movement
		bool directionChanged = (currentDeltaRow != nextDeltaRow) || (currentDeltaCol != nextDeltaCol);
		bool isFirstCell = (i == 0);
		bool isSecondToLastCell = (i == inCellPath.path.size() - 2);
		// Always move to the last cell of the path
		// If the direction has changed or it's the final cell, perform a mouse movement
		if (isFirstCell || isSecondToLastCell || directionChanged || streak > 2) {
			
			int newCellX = std::round(((double)(currentCell.col + 0.5) / inGridInfo.cols) * gridWidth) + inGridRect.left;
			int newCellY = std::round(((double)(currentCell.row + 0.5) / inGridInfo.rows) * gridHeight) + inGridRect.top;

			// Add a small delay to allow the OS to catch up
			MouseMove(newCellX, newCellY);
			std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
			if (isFirstCell)
				mouse_event(MOUSEEVENTF_LEFTDOWN, 1, 0, 0, 0);
			// Update the direction for the next iteration
			currentDeltaRow = nextDeltaRow;
			currentDeltaCol = nextDeltaCol;
			streak = 0;
			continue;
		}
		streak++;
	}
	Cell lastCell = inCellPath.path.back();

	int cellX = std::round(((double)(lastCell.col + 0.5) / inGridInfo.cols) * gridWidth) + inGridRect.left;
	int cellY = std::round(((double)(lastCell.row + 0.5) / inGridInfo.rows) * gridHeight) + inGridRect.top;
	MouseMove(cellX, cellY);
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
	// Release the mouse button at the end of the path
	mouse_event(MOUSEEVENTF_LEFTUP, 1, 0, 0, 0);
	std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
}

void solvingSequence() {
	PixelReader pixelReader;
	RECT gridRect;
	GridInfo gridInfo;
	std::vector<NodePair> nodePairs;
	bool success;
	success = getGridClientPosition(gridRect);
	if (!success) {
		std::cout << "FAILED! #1" << std::endl;
		return;
	}
	success = getGridCellDimensions(pixelReader, gridRect, gridInfo);
	if (!success) {
		std::cout << "FAILED! #2" << std::endl;
		return;
	}
	success = getNodePairs(pixelReader, gridRect, gridInfo, nodePairs);
	if (!success) {
		std::cout << "FAILED! #3" << std::endl;
		return;
	}

	GeneratorSolver generatorSolver(nodePairs, gridInfo.rows, gridInfo.cols);
	success = generatorSolver.solve();
	if (!success) {
		std::cout << "FAILED! #4" << std::endl;
		return;
	}
	for (int row = 0; row < gridInfo.rows; row++) {
		for (int col = 0; col < gridInfo.cols; col++) {
			std::string gridValue(1, generatorSolver.getSolvedGridValue(row, col));
			std::cout << gridValue <<  " ";
		}
		std::cout << std::endl;
	}
	std::vector<CellPath> cellPaths = generatorSolver.getCellPaths();
	for (CellPath cellPath : cellPaths) {
		performThings(cellPath, gridRect, gridInfo);
	}
}
int main() {
	std::cout << "Make roblox the active window so the macro can see the puzzle clearly!";
	
	bool isKeyPressed = false;

	while (true) {
		// Check if the 'A' key (virtual key code 0x41) is pressed.
		// The high-order bit is set if the key is down.
		if (GetAsyncKeyState(0x46) & 0x8000) { // hold F;
			solvingSequence();
				
		}
		else {
			isKeyPressed = false;
		}

		// A short delay to prevent the loop from consuming too much CPU.
		Sleep(10);
	}
		
	std::this_thread::sleep_for(std::chrono::seconds(1000));

	return 0;
}