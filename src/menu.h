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
} MenuValue;

typedef struct {
    MenuValue * element; // TODO: Rename to menuValue eller value
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
    bool isLeaf;
} MenuViewModel;

// Callback function pointers:
typedef void (*OnMenuValueChanged)(MenuValue menuValue, int oldValue);
typedef void (*OnAction)(MenuNode menuNode);
typedef void (*ValueRenderer)(MenuNode menuNode, int value, bool verbose, char * target);

typedef struct {
    MenuValue * elements;
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
    ValueRenderer valueRenderer;
} Menu;

Menu createMenu(
    int pageSize,
    OnMenuValueChanged onMenuValueChanged,
    OnAction onAction,
    ValueRenderer valueRenderer
);

int createMenuLeaf(
    Menu * menu,
    const char * name,
    const char * description,
    int minimumValue,
    int maximumValue,
    int * valuePointer
);

int createMenuNonLeaf(
    Menu * menu,
    const char * name,
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