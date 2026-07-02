#include <string.h>
#include <stdio.h>

#include "../src/menu.h"

#define BOX_WIDTH 7

void renderMenu(Menu * menu, char * targetLineTop, char * targetLineBottom, int maxLineLength) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    if (openMenuNode->nofChildren == 0) {
        // TODO: Bruk to bokser i stedet for to linjer, og så kan man velge hvordan man rendrer det
        snprintf(targetLineTop, maxLineLength, "%s", menu->viewModel.boxes[0].text);
        snprintf(targetLineBottom, maxLineLength, "%s", menu->viewModel.boxes[0].valueText);
    } else {

        // Clear lines first:
        for (int i = 0; i < maxLineLength; i++) {
            targetLineTop[i] = ' ';
            targetLineBottom[i] = ' ';
        }

        int i = 0;

        if (!menu->viewModel.isFirstPage) {
            targetLineTop[i] = '<';
            i += 2;
        }
        
        for (int boxIndex = 0; boxIndex < menu->viewModel.nofBoxes; boxIndex++) {
            bool isSelected = menu->viewModel.boxes[boxIndex].isSelected;

            if (isSelected) {
                targetLineTop[i] = '|';
                targetLineBottom[i] = '|';
                i += 2;
            }
            
            int j = 0;
            int k = i;
            char * targetLine = targetLineTop;
            for (; j < maxLineLength; j++) {
                char c = menu->viewModel.boxes[boxIndex].text[j];
                if (c == 0) {
                    break;
                } else if (c == '\n') {
                    targetLine = targetLineBottom;
                    k = i;
                } else if (c == '\x1A') {
                    int len = strlen(menu->viewModel.boxes[boxIndex].valueText);
                    memcpy(targetLine + k, menu->viewModel.boxes[boxIndex].valueText, len);
                    k += len;
                } else {
                    targetLine[k++] = c;
                }
            }

            i += BOX_WIDTH + 1;
        }

        if (!menu->viewModel.isLastPage) {
            targetLineTop[i+1] = '>';
            i += 2;
        }

        targetLineTop[i] = 0;
        targetLineBottom[i] = 0;
    }
}