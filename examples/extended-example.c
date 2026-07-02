#include <stdio.h>
#include <string.h>

#include "render-to-string.h"
#include "../src/menu.h"
// TODO: Skal vel være #include <headless_menu.h> fra libs

typedef struct {
    int channel;
    int brightness;

    int enabled;
    int note;
    
    int envelopeType;
    int attack;
    int decay;
    int sustain;
    int release;
} Settings;


typedef struct {
    int paramA;
    int paramB;
} Data;


Menu menu; // TODO: Gjøre createMenu her?

Settings settings = {0};

Data data[8] = {0};
int nofData = 0;
int dataParentMenuIndex;

#define MAX_LINE_CHARACTERS 32 // Need at least 7 * boxes + 6 for markers and + 1 for null terminator

char lineTop[MAX_LINE_CHARACTERS];
char lineBottom[MAX_LINE_CHARACTERS];

void onMenuValueChanged(MenuValue element, int oldValue) {
    if (strcmp(element.description, "MIDI CHANNEL") == 0) {
        printf("MIDI channel changed from %d to %d\n", oldValue, *element.valuePointer);
    } else if (strcmp(element.description, "DRONE NOTE") == 0) {
        printf("Drone note changed from %d to %d\n", oldValue, *element.valuePointer);
    }
}

void addNewMenuElement() {
    if (nofData >= 8) {
        printf("Max number of data elements reached\n");
        return;
    }

    int childIndexA = createMenuLeaf(&menu, "PARAM A\n\x1A", "PARAMETER A", 0, 8, &data[nofData].paramA, NULL, NULL);
    int childIndexB = createMenuLeaf(&menu, "PARAM B\n\x1A", "PARAMETER B", 0, 127, &data[nofData].paramB, NULL, NULL);
    int deleteIndex = createMenuAction(&menu, "DEL\nETE");
    const char * children[] = {"PARAM A\n\x1A", "PARAM B\n\x1A", "DEL\nETE"};
    int parentIndex = createMenuNonLeaf(&menu, "DATA", children, 3);
    connectMenuChild(&menu, dataParentMenuIndex, parentIndex);
    nofData++;
}

void onAction(MenuNode node) {
    if (strcmp(node.name, "ADD\nDATA") == 0) {
        addNewMenuElement();
    } else if (strcmp(node.name, "ACTION") == 0) {
        printf("Performing action...\n");
    } else if (strcmp(node.name, "DEL\nETE") == 0) {
        removeMenuChild(&menu, dataParentMenuIndex, node.parentIndex);
    }
}

void setupMenu() {
    menu = createMenu(3, onMenuValueChanged, onAction);

    const char * yesNoNames[] = {"NO", "YES"};
    const char * highLowNames[] = {"LOW", "HIGH"};
    const char * onOffNames[] = {"OFF", "ON"};

    createMenuLeaf(&menu, "STATE\n\x1A", "DRONE ENABLED", 0, 1, &settings.enabled, onOffNames, onOffNames);
    createMenuLeaf(&menu, "NOTE\n\x1A", "DRONE NOTE", 0, 127, &settings.note, NULL, NULL);
    const char * droneChildren[] = {"STATE\n\x1A", "NOTE\n\x1A"};
    createMenuNonLeaf(&menu, "DRONE", droneChildren, 2);

    const char * envTypeNames[] = {"ADSR", "AR"};
    createMenuLeaf(&menu, "TYPE\n\x1A", "ENVELOPE TYPE", 0, 1, &settings.envelopeType, envTypeNames, envTypeNames);
    createMenuLeaf(&menu, "ATTACK\n\x1A", "ENVELOPE ATTACK TIME", 0, 1000, &settings.attack, NULL, NULL);
    createMenuLeaf(&menu, "DECAY\n\x1A", "ENVELOPE DECAY TIME", 0, 1000, &settings.decay, NULL, NULL);
    createMenuLeaf(&menu, "SUST\n\x1A", "ENVELOPE SUSTAIN", 0, 100, &settings.sustain, NULL, NULL);
    createMenuLeaf(&menu, "RELE\n\x1A", "ENVELOPE RELEASE TIME", 0, 1000, &settings.release, NULL, NULL);
    const char * envelopeChildren[] = {"TYPE\n\x1A", "ATTACK\n\x1A", "DECAY\n\x1A", "SUST\n\x1A", "RELE\n\x1A"};
    createMenuNonLeaf(&menu, "ENVE\nLOPE", envelopeChildren, 5);

    createMenuLeaf(&menu, "CHAN\n\x1A", "MIDI CHANNEL", 1, 16, &settings.channel, NULL, NULL);
    createMenuLeaf(&menu, "BRIGHT\n\x1A", "DISPLAY BRIGHTNESS", 0, 1, &settings.brightness, highLowNames, highLowNames);
    createMenuAction(&menu, "ADD\nDATA");
    const char * systemChildren[] = {"CHAN\n\x1A", "BRIGHT\n\x1A", "ADD\nDATA"};
    dataParentMenuIndex = createMenuNonLeaf(&menu, "SYSTEM", systemChildren, 3);

    createMenuAction(&menu, "ACTION");

    const char * menuChildren[] = {"ENVE\nLOPE", "DRONE", "ACTION", "SYSTEM"};
    createMenuRoot(&menu, menuChildren, 4);
}

char readInput() {
    printf("\nEnter commands ([a] left, [d] right, [w] into, [s] back, [q] quit): ");
    char command = 0;
    while (command < 'a' || command > 'z') {
        command = getchar();
    }
    return command;
}

int doCommands(Menu * menu, char * commands) {
    int i = 0;
    char command = commands[i];

    while (command != 0) {
        if (command == 'a') {
            goLeftMenu(menu);
        } else if (command == 'd') {
            goRightMenu(menu);
        } else if (command == 'w') {
            goIntoMenu(menu);
        } else if (command == 's') {
            goBackMenu(menu);
        } else if (command == 'q') {
            return 0;
        }

        command = commands[++i];
    }
    
    return 1;
}

int loop() {
    renderMenu(menu.viewModel, lineTop, lineBottom, MAX_LINE_CHARACTERS);

    printf("------\n");
    printf("%s\n", lineTop);
    printf("%s\n", lineBottom);

    char commands[2];
    commands[0] = readInput();
    commands[1] = 0;

    return doCommands(&menu, commands);
}

void assertEquals(char actual, char expected) {
    if (actual != expected) {
        printf("\n|\n|\nERROR: Assert failed: %c != %c\n|\n|\n", actual, expected);
        //exit(EXIT_FAILURE);
    }
}

void doTest() {
    // TEST: Delete menu item:
    doCommands(&menu, "ddddw"); // SYSTEM
    doCommands(&menu, "ddww"); // 2X ADD DATA
    doCommands(&menu, "ddw"); // DATA (nr 2)
    doCommands(&menu, "dwdddddss"); // 5X PARAM B
    doCommands(&menu, "aw"); // DATA (nr 1)
    doCommands(&menu, "ddw"); // DELETE
    doCommands(&menu, "dw"); // DATA (nr 2)

    assertEquals(menu.viewModel.boxes[1].valueText[0], '5');
    renderMenu(menu.viewModel, lineTop, lineBottom, MAX_LINE_CHARACTERS);
    assertEquals(lineBottom[8 + 2], '5'); // + 2 because of the "| " at the start of the line
}

void setup() {
    setupMenu();
    doTest();
}

int main() {
    setup();
    while (loop()) {}
    return 0;
}