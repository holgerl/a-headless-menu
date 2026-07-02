#include <string.h>
#include <stdio.h>

#include "../src/menu.h"

void renderMenu(Menu * menu, char * targetLineTop, char * targetLineBottom, int maxLineLength) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    if (openMenuNode->nofChildren == 0) {
        snprintf(targetLineTop, maxLineLength, "%s", menu->viewModel.boxes[0].lineTop);
        snprintf(targetLineBottom, maxLineLength, "%s", menu->viewModel.boxes[0].lineBottom);
    } else {
        char * current = targetLineTop;
        int max = maxLineLength;

        if (!menu->viewModel.isFirstPage) {
            current += snprintf(current, max, "< ");
            max -= 2;
        }
        
        for (int i = 0; i < menu->viewModel.nofBoxes; i++) {
            bool isSelected = menu->viewModel.boxes[i].isSelected;

            if (isSelected) {
                current += snprintf(current, max, "| ");
                max -= 2;
            }
            current += snprintf(current, max, "%-7s", menu->viewModel.boxes[i].lineTop);
            max -= 7;
            current += snprintf(current, max, " ");
            max -= 1;
        }

        if (!menu->viewModel.isLastPage) {
            current += snprintf(current, max, ">");
            max -= 1;
        }

        current = targetLineBottom;
        max = maxLineLength;

        if (!menu->viewModel.isFirstPage) {
            current += snprintf(current, max, "  ");
            max -= 2;
        }

        for (int i = 0; i < menu->viewModel.nofBoxes; i++) {
            bool isSelected = menu->viewModel.boxes[i].isSelected;
            if (isSelected) {
                current += snprintf(current, max, "| ");
                max -= 2;
            }
            current += snprintf(current, max, "%-7s", menu->viewModel.boxes[i].lineBottom);
            max -= 7;
            current += snprintf(current, max, " ");
            max -= 1;
        }
    }
}