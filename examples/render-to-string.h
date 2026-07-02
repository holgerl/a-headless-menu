#include <string.h>
#include <stdio.h>

#include "../src/menu.h"

#define BOX_WIDTH 7

void renderMenu(MenuViewModel viewModel, char * targetLineTop, char * targetLineBottom, int maxLineLength) {
    if (viewModel.isLeaf) {
        snprintf(targetLineTop, maxLineLength, "%s", viewModel.boxes[0].text);
        snprintf(targetLineBottom, maxLineLength, "%s", viewModel.boxes[0].valueText);
    } else {
        // Clear lines first:
        for (int i = 0; i < maxLineLength; i++) {
            targetLineTop[i] = ' ';
            targetLineBottom[i] = ' ';
        }

        int i = 0;

        if (!viewModel.isFirstPage) {
            targetLineTop[i] = '<';
            i += 2;
        }
        
        for (int boxIndex = 0; boxIndex < viewModel.nofBoxes; boxIndex++) {
            bool isSelected = viewModel.boxes[boxIndex].isSelected;

            if (isSelected) {
                targetLineTop[i] = '|';
                targetLineBottom[i] = '|';
                i += 2;
            }
            
            int k = i;
            char * targetLine = targetLineTop;
            for (int j = 0; j < maxLineLength; j++) {
                char c = viewModel.boxes[boxIndex].text[j];
                if (c == 0) {
                    break;
                } else if (c == '\n') {
                    targetLine = targetLineBottom;
                    k = i;
                } else {
                    targetLine[k++] = c;
                }
            }

            i += BOX_WIDTH + 1;
        }

        if (!viewModel.isLastPage) {
            targetLineTop[i+1] = '>';
            i += 2;
        }

        targetLineTop[i] = 0;
        targetLineBottom[i] = 0;
    }
}