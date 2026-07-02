#ifndef menu_h
#define menu_h

#include <stdbool.h>

// Indices refer to the nodes array except when stated

typedef struct {
    const char * description;

    int minimumValue;
    int maximumValue;
    int * valuePointer;

    const char * * shortValueNames;
    const char * * longValueNames;
} MenuElement;

typedef struct {
    MenuElement * element;
    const char * name;
    int * childrenIndices;
    int nofChildren;
    int index;
    int parentIndex;
    int selectedIndex; // index of the focused child in childrenIndices (not the nodes array)
    bool isAction;
} MenuNode;

typedef struct {
    char * text;
    char * valueText;
    bool isSelected;
} MenuBox;

typedef struct {
    MenuBox * boxes;
    int nofBoxes;
    bool isFirstPage;
    bool isLastPage;
} MenuViewModel;

// Callback function pointers:
typedef void (*OnMenuValueChanged)(MenuElement menuElement, int oldValue);
typedef void (*OnAction)(MenuNode menuNode);

typedef struct {
    MenuElement * elements;
    MenuNode * nodes;
    int nofElements;
    int nofNodes;
    int rootIndex;
    int openIndex;
    int pageSize;
    bool bigIncrements;
    MenuViewModel viewModel;
    OnMenuValueChanged onMenuValueChanged;
    OnAction onAction;
} Menu;

Menu createMenu(
    int pageSize,
    OnMenuValueChanged onMenuValueChanged,
    OnAction onAction
);

int createMenuLeaf(
    Menu * menu,
    const char * name,
    const char * description,
    int minimumValue,
    int maximumValue,
    int * valuePointer,
    const char * * shortValueNames,
    const char * * longValueNames
);

int createMenuNonLeaf(
    Menu * menu,
    const char * mame,
    const char * * childrenNames,
    int nofChildren
);

int createMenuAction(
    Menu * menu,
    const char * name
);

int createMenuRoot(
    Menu * menu,
    const char * * childrenNames,
    int nofChildren
);

void connectMenuChild(
    Menu * menu,
    int parentIndex,
    int childIndex
);

void removeMenuChild(
    Menu * menu,
    int parentIndex,
    int childIndex
);

const char * getOpenMenuNodeName(Menu * menu);

void goIntoMenu(Menu * menu);
void goBackMenu(Menu * menu);
void goRightMenu(Menu * menu);
void goLeftMenu(Menu * menu);

void openMenuNode(Menu * menu, const char * name);

void updateMenuViewModel(Menu * menu);

#endif