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

void addNewMenuElement() {
    if (nofData >= 8) {
        printf("Max number of data elements reached\n");
        return;
    }

    int childIndexA = createMenuLeaf(&menu, "ID\n%s", "Identity", 0, 8, &data[nofData].paramA);
    int childIndexB = createMenuLeaf(&menu, "PARAM\n%s", "PARAMETER", 0, 127, &data[nofData].paramB);
    int deleteIndex = createMenuAction(&menu, "DEL\nETE");
    const char * children[] = {"ID\n%s", "PARAM\n%s", "DEL\nETE"};
    int parentIndex = createMenuNonLeaf(&menu, "DATA\n%s", children, 3);
    connectMenuChild(&menu, dataParentMenuIndex, parentIndex);
    nofData++;
}

void onMenuValueChanged(MenuValue menuValue, int oldValue) {
    if (strcmp(menuValue.description, "MIDI CHANNEL") == 0) {
        printf("MIDI channel changed from %d to %d\n", oldValue, *menuValue.valuePointer);
    } else if (strcmp(menuValue.description, "DRONE NOTE") == 0) {
        printf("Drone note changed from %d to %d\n", oldValue, *menuValue.valuePointer);
    }
}

void onAction(MenuNode node) {
    if (strcmp(node.name, "ADD\nDATA") == 0) {
        addNewMenuElement();
    } else if (strcmp(node.name, "ACTION") == 0) {
        printf("Performing action...\n");
    } else if (strcmp(node.name, "DEL\nETE") == 0) {
        int dataMenuIndex = node.parentIndex;
        goBackMenu(&menu);
        goLeftMenu(&menu);
        removeMenuChild(&menu, dataParentMenuIndex, dataMenuIndex);
    }
}

void valueRenderer(MenuNode menuNode, int value, bool verbose, char * target) {
    const char * envTypeShortNames[] = {"ADSR", "AR", "HOLD", "OPEN"};
    const char * envTypeLongNames[] = {"Atk-Decay-Sust-Rel", "Attack-Release", "Hold-Release", "Open"};

    // Using == here because it is fast and works in this file:
    if (menuNode.name == "NOTE\n%s") {
        sprintf(target, "note #%d", value);
    } else if (menuNode.name == "STATE\n%s") {
        sprintf(target, "%s", value ? "ON" : "OFF");
    } else if (menuNode.name == "TYPE\n%s") {
        sprintf(target, "%s", verbose ? envTypeLongNames[value] : envTypeShortNames[value]);
    } else if (menuNode.name == "BRIGHT\n%s") {
        sprintf(target, "%s", value ? "HIGH" : "LOW");
    } else if (menuNode.name == "DATA\n%s") {
        int value = *(menu.nodes[menuNode.childrenIndices[0]].element->valuePointer);
        sprintf(target, "%d", value);
    } else {
        sprintf(target, "%d", value);
    }
}

void setupMenu() {
    menu = createMenu(3, onMenuValueChanged, onAction, valueRenderer);

    const char * highLowNames[] = {"LOW", "HIGH"};

    createMenuLeaf(&menu, "STATE\n%s", "DRONE ENABLED", 0, 1, &settings.enabled);
    createMenuLeaf(&menu, "NOTE\n%s", "DRONE NOTE", 0, 127, &settings.note);
    const char * droneChildren[] = {"STATE\n%s", "NOTE\n%s"};
    createMenuNonLeaf(&menu, "DRONE", droneChildren, 2);

    createMenuLeaf(&menu, "TYPE\n%s", "ENVELOPE TYPE", 0, 3, &settings.envelopeType);
    createMenuLeaf(&menu, "ATTACK\n%s", "ENVELOPE ATTACK TIME", 0, 1000, &settings.attack);
    createMenuLeaf(&menu, "DECAY\n%s", "ENVELOPE DECAY TIME", 0, 1000, &settings.decay);
    createMenuLeaf(&menu, "SUST\n%s", "ENVELOPE SUSTAIN", 0, 100, &settings.sustain);
    createMenuLeaf(&menu, "RELE\n%s", "ENVELOPE RELEASE TIME", 0, 1000, &settings.release);
    const char * envelopeChildren[] = {"TYPE\n%s", "ATTACK\n%s", "DECAY\n%s", "SUST\n%s", "RELE\n%s"};
    createMenuNonLeaf(&menu, "ENVE\nLOPE", envelopeChildren, 5);

    createMenuLeaf(&menu, "CHAN\n%s", "MIDI CHANNEL", 1, 16, &settings.channel);
    createMenuLeaf(&menu, "BRIGHT\n%s", "DISPLAY BRIGHTNESS", 0, 1, &settings.brightness);
    createMenuAction(&menu, "ADD\nDATA");
    const char * systemChildren[] = {"CHAN\n%s", "BRIGHT\n%s", "ADD\nDATA"};
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