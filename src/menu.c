#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"

#define MAX_MENU_SIZE 64
#define MAX_LINE_CHARACTERS 32

const char * getShortValueName(MenuElement * element);

Menu createMenu(int pageSize, OnMenuValueChanged onMenuValueChanged, OnAction onAction) {
    MenuElement * elements = (MenuElement *) malloc(MAX_MENU_SIZE * sizeof(MenuElement));
    MenuNode * nodes = (MenuNode *) malloc(MAX_MENU_SIZE * sizeof(MenuNode));

    if (elements == NULL) { printf("%s\n", "ERROR: elements == NULL"); }
    if (nodes == NULL) { printf("%s\n", "ERROR: nodes == NULL"); }

    MenuBox * boxes = (MenuBox *) malloc(pageSize * sizeof(MenuBox));
    if (boxes == NULL) { printf("%s\n", "ERROR: boxes == NULL"); }
    for (int i = 0; i < pageSize; i++) {
        boxes[i].text = (char *) malloc(MAX_LINE_CHARACTERS * sizeof(char));
        if ( boxes[i].text == NULL) { printf("%s\n", "ERROR: boxes[i].text == NULL"); }
        boxes[i].valueText = (char *) malloc(MAX_LINE_CHARACTERS * sizeof(char));
        if ( boxes[i].valueText == NULL) { printf("%s\n", "ERROR: boxes[i].valueText == NULL"); }
    }
    MenuViewModel viewModel = {boxes, -1, false, false};

    Menu menu = {elements, nodes, 0, 0, 0, 0, pageSize, false, viewModel, onMenuValueChanged, onAction};

    return menu;
}

int getMenuNodeIndexByName(Menu * menu, const char * name) {
    for (int i = menu->nofNodes - 1; i >= 0; i--) { // Search most recently added nodes first, in case of duplicate names
        if (strcmp(menu->nodes[i].name, name) == 0) {
            return i;
        }
    }

    printf("\n\nERROR: Found no menu node with name %s\n\n", name);
    //exit(EXIT_FAILURE);
    return -1;
}

int createMenuLeaf(
    Menu * menu,
    const char * name,
    const char * description,
    int minimumValue,
    int maximumValue,
    int * valuePointer,
    const char * * shortValueNames,
    const char * * longValueNames
) {
    if (menu->nofElements >= MAX_MENU_SIZE || menu->nofNodes >= MAX_MENU_SIZE) {
        printf("%s\n", "ERROR: Maximum menu size exceeded");
        //exit(EXIT_FAILURE);
    }

    const char * * shortValueNamesCopy = NULL;
    const char * * longValueNamesCopy = NULL;
    if (shortValueNames != NULL) {
        int nameArrayBytes = (maximumValue - minimumValue + 1) * sizeof(const char *);
        shortValueNamesCopy = (const char * *) malloc(nameArrayBytes);
        if (shortValueNamesCopy == NULL) { printf("%s\n", "ERROR: shortValueNamesCopy == NULL"); }
        longValueNamesCopy = (const char * *) malloc(nameArrayBytes);
        if (longValueNamesCopy == NULL) { printf("%s\n", "ERROR: longValueNamesCopy == NULL"); }
        memcpy(shortValueNamesCopy, shortValueNames, nameArrayBytes);
        memcpy(longValueNamesCopy, longValueNames, nameArrayBytes);
    }

    MenuElement element = {
        .description = description,
        .minimumValue = minimumValue,
        .maximumValue = maximumValue,
        .valuePointer = valuePointer,
        .shortValueNames = shortValueNamesCopy,
        .longValueNames = longValueNamesCopy
    };

    menu->elements[menu->nofElements] = element;

    int newIndex = menu->nofNodes;

    MenuNode node = {
        .element = &menu->elements[menu->nofElements],
        .name = name,
        .childrenIndices = NULL,
        .nofChildren = 0,
        .index = newIndex,
        .parentIndex = -1,
        .selectedIndex = -1,
        .isAction = false
    };

    menu->nodes[newIndex] = node;

    menu->nofElements++;
    menu->nofNodes++;

    //printf("%s\n", "createMenuLeaf X"); Serial.flush();

    return newIndex;
}

int createMenuNonLeaf(
    Menu * menu,
    const char * name,
    const char * * childrenNames,
    int nofChildren
) {
    if (menu->nofNodes >= MAX_MENU_SIZE) {
        printf("%s\n", "ERROR: Maximum menu size exceeded");
        //exit(EXIT_FAILURE);
    }

    int newIndex = menu->nofNodes;

    int * childrenIndices = (int *) malloc(nofChildren * sizeof(int));

    if (childrenIndices == NULL) { printf("\n%s\n\n", "ERROR: childrenIndices == NULL"); }
        
    for (int i = 0; i < nofChildren; i++) {
        int childIndex = getMenuNodeIndexByName(menu, childrenNames[i]); // TODO: Dette gjør det umulig å ha barn med samme navn som allerede finnes
        childrenIndices[i] = childIndex;
        menu->nodes[childIndex].parentIndex = newIndex;
    }

    MenuNode node = {
        .element = NULL,
        .name = name,
        .childrenIndices = childrenIndices,
        .nofChildren = nofChildren,
        .index = newIndex,
        .parentIndex = -1, // Will be set later
        .selectedIndex = 0,
        .isAction = false
    };

    menu->nodes[newIndex] = node;

    menu->nofNodes++;

    return newIndex;
}

int createMenuAction(
    Menu * menu,
    const char * name
) {
    if (menu->nofNodes >= MAX_MENU_SIZE) {
        printf("%s\n", "ERROR: Maximum menu size exceeded");
        //exit(EXIT_FAILURE);
    }

    int newIndex = menu->nofNodes;

    MenuNode node = {
        .element = NULL,
        .name = name,
        .childrenIndices = NULL,
        .nofChildren = 0,
        .index = newIndex,
        .parentIndex = -1, // Will be set later
        .selectedIndex = 0,
        .isAction = true
    };

    menu->nodes[newIndex] = node;

    menu->nofNodes++;

    return newIndex;
}

int createMenuRoot(
    Menu * menu,
    const char * * childrenNames,
    int nofChildren
) {
    int rootIndex = createMenuNonLeaf(menu, "Root", childrenNames, nofChildren);
    menu->rootIndex = rootIndex;
    menu->openIndex = menu->rootIndex;

    updateMenuViewModel(menu);

    return rootIndex;
}

void connectMenuChild(
    Menu * menu,
    int parentIndex,
    int childIndex
) {
    MenuNode * parentNode = &menu->nodes[parentIndex];

    parentNode->childrenIndices = (int *) realloc(
        parentNode->childrenIndices,
        (parentNode->nofChildren + 1) * sizeof(int)
    );

    parentNode->childrenIndices[parentNode->nofChildren] = childIndex;
    parentNode->nofChildren++;

    menu->nodes[childIndex].parentIndex = parentIndex;

    updateMenuViewModel(menu);
}

void freeMenuNodeRecursively(Menu * menu, int nodeIndex) {
    MenuNode node = menu->nodes[nodeIndex];

    if (node.element != NULL) {
        free(node.element->shortValueNames);
        free(node.element->longValueNames);
    }

    for (int i = 0; i < node.nofChildren; i++) {
        freeMenuNodeRecursively(menu, node.childrenIndices[i]);
    }

    free(node.childrenIndices);
}

void removeMenuChild(
    Menu * menu,
    int parentIndex,
    int childIndex
) {
    MenuNode * parentNode = &menu->nodes[parentIndex];

    freeMenuNodeRecursively(menu, childIndex);

    int foundAtIndex = -1;
    for (int i = 0; i < parentNode->nofChildren; i++) {
        if (parentNode->childrenIndices[i] == childIndex) {
            foundAtIndex = i;
            break;
        }
    }

    if (foundAtIndex == -1) {
        printf("ERROR: Tried to remove child %d from parent %d, but child was not found among parent's children\n", childIndex, parentIndex);
        //exit(EXIT_FAILURE);
    }

    if (menu->openIndex == childIndex) {
        goBackMenu(menu);
        goLeftMenu(menu);
    }

    for (int i = foundAtIndex; i < parentNode->nofChildren - 1; i++) {
        parentNode->childrenIndices[i] = parentNode->childrenIndices[i + 1];
    }

    parentNode->childrenIndices = (int *) realloc(
        parentNode->childrenIndices,
        (parentNode->nofChildren - 1) * sizeof(int)
    );

    parentNode->nofChildren--;

    updateMenuViewModel(menu);
}

void goIntoMenu(Menu * menu) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    if (openMenuNode->nofChildren > 0) {
        int selectedNodeIndex = openMenuNode->childrenIndices[openMenuNode->selectedIndex];
        MenuNode selectedNode = menu->nodes[selectedNodeIndex];
        
        if (selectedNode.isAction) {
            menu->onAction(selectedNode);
        } else {
            menu->openIndex = selectedNodeIndex;
            menu->nodes[menu->openIndex].selectedIndex = 0;
        }
    } else {
        int range = openMenuNode->element->maximumValue - openMenuNode->element->minimumValue;
        if (range >= 20) {
            menu->bigIncrements = !menu->bigIncrements;
        }
    }

    updateMenuViewModel(menu);
}

void goBackMenu(Menu * menu) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    if (menu->openIndex != menu->rootIndex) {
        menu->openIndex = openMenuNode->parentIndex;
    }

    menu->bigIncrements = false;

    updateMenuViewModel(menu);
}

void goLeftRightMenu(Menu * menu, int increment) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    if (openMenuNode->nofChildren > 0) {
        if (openMenuNode->selectedIndex + increment >= 0
            && openMenuNode->selectedIndex + increment < openMenuNode->nofChildren) {
            openMenuNode->selectedIndex += increment;
        }
    } else {
        int currentValue = *(openMenuNode->element->valuePointer);
        int actualIncrement = increment;

        if (menu->bigIncrements) {
            int range = openMenuNode->element->maximumValue - openMenuNode->element->minimumValue;
            if (range == 255 || range == 127 || range == 63) {
                actualIncrement = increment * 16;
            } else {
                actualIncrement = increment * 10;
            }
        }

        int newValue = currentValue + actualIncrement;

        if (newValue < openMenuNode->element->minimumValue) {
            newValue = openMenuNode->element->minimumValue;
        }

        if (newValue > openMenuNode->element->maximumValue) {
            newValue = openMenuNode->element->maximumValue;
        }

        *(openMenuNode->element->valuePointer) = newValue;

        if (newValue != currentValue) {
            menu->onMenuValueChanged(*openMenuNode->element, currentValue);
        }
    }

    updateMenuViewModel(menu);
}

void goLeftMenu(Menu * menu) {
    goLeftRightMenu(menu, -1);
}

void goRightMenu(Menu * menu) {
    goLeftRightMenu(menu, 1);
}

void openMenuNode(Menu * menu, const char * name) {
    int nodeIndex = getMenuNodeIndexByName(menu, name);
    menu->openIndex = nodeIndex;
    menu->nodes[menu->openIndex].selectedIndex = 0;
    updateMenuViewModel(menu);
}

const char * getOpenMenuNodeName(Menu * menu) {
    return menu->nodes[menu->openIndex].name;
}

const char * getShortValueName(MenuElement * element) {
    int value = *(element->valuePointer);
    int nameIndex = value - element->minimumValue;
    const char * shortValueName = element->shortValueNames[nameIndex];

    return shortValueName;
}

void updateMenuViewModel(Menu * menu) {
    MenuNode * openMenuNode = &menu->nodes[menu->openIndex];

    int firstIndexInPage = (openMenuNode->selectedIndex / menu->pageSize) * (menu->pageSize);
    menu->viewModel.isFirstPage = openMenuNode->selectedIndex < menu->pageSize;
    menu->viewModel.isLastPage = openMenuNode->nofChildren - firstIndexInPage <= menu->pageSize;

    if (openMenuNode->nofChildren == 0) {
        MenuElement * openMenuElement = openMenuNode->element;
        int value = *(openMenuElement->valuePointer);

        strcpy(menu->viewModel.boxes[0].text, openMenuElement->description);

        const char * prefix = menu->bigIncrements ? "<<<" : "<";
        const char * postfix = menu->bigIncrements ? ">>>" : ">";

        if (openMenuElement->shortValueNames != NULL) {
            int nameIndex = value - openMenuElement->minimumValue;
            const char * name = openMenuElement->longValueNames[nameIndex];
            sprintf(menu->viewModel.boxes[0].valueText, "%s  %s  %s", prefix, name, postfix);
        } else {
            sprintf(menu->viewModel.boxes[0].valueText, "%s  %d  %s", prefix, value, postfix);
        }

        menu->viewModel.nofBoxes = menu->pageSize;
    } else {
        int firstIndexInPage = (openMenuNode->selectedIndex / menu->pageSize) * menu->pageSize;
        bool isFirstPage = openMenuNode->selectedIndex < menu->pageSize;
        bool isLastPage = firstIndexInPage + menu->pageSize >= openMenuNode->nofChildren;

        int nofElementsOnPage = isLastPage ?
            openMenuNode->nofChildren - firstIndexInPage :
            menu->pageSize;

        for (int i = 0; i < nofElementsOnPage; i++) {
            int nodeIndex = firstIndexInPage + i;

            MenuNode * childMenuNode = &menu->nodes[openMenuNode->childrenIndices[nodeIndex]];
            MenuElement * childMenuElement = childMenuNode->element;

            menu->viewModel.boxes[i].isSelected = nodeIndex == openMenuNode->selectedIndex;

            strcpy(menu->viewModel.boxes[i].text, childMenuNode->name);

            if (!childMenuNode->isAction && childMenuNode->nofChildren == 0) {
                if (childMenuElement->shortValueNames != NULL) {
                    strcpy(menu->viewModel.boxes[i].valueText, getShortValueName(childMenuElement));
                } else {
                    int childValue = *(childMenuElement->valuePointer);
                    sprintf(menu->viewModel.boxes[i].valueText, "%d", childValue);
                }
            } 
        }

        menu->viewModel.nofBoxes = nofElementsOnPage;
    }
    
    return;
}