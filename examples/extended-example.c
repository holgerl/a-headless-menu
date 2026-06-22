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

Menu menuX; // TODO: Gjøre createMenu her?
Settings settings = {0};

char lineTop[64]; // Need extra bytes for color esape codes
char lineBottom[64];

void onMenuValueChanged(MenuElement element, int oldValue) {
    if (strcmp(element.description, "MIDI CHANNEL") == 0) {
        printf("MIDI channel changed from %d to %d\n", oldValue, *element.valuePointer);
    } else if (strcmp(element.description, "DRONE NOTE") == 0) {
        printf("Drone note changed from %d to %d\n", oldValue, *element.valuePointer);
    }
}

void onAction(MenuNode node) {
    if (strcmp(node.name, "CLEARDATA") == 0) {
        printf("Clearing data...\n");
    } else if (strcmp(node.name, "ACTION") == 0) {
        printf("Performing action...\n");
    }
}

void setupMenu() {
    menuX = createMenu(3, onMenuValueChanged, onAction);

    const char * yesNoNames[] = {"NO", "YES"};
    const char * highLowNames[] = {"LOW", "HIGH"};
    const char * onOffNames[] = {"OFF", "ON"};

    createMenuLeaf(&menuX, "STATE", "DRONE ENABLED", 0, 1, &settings.enabled, onOffNames, onOffNames);
    createMenuLeaf(&menuX, "NOTE", "DRONE NOTE", 0, 127, &settings.note, NULL, NULL);
    const char * droneChildren[] = {"STATE", "NOTE"};
    createMenuNonLeaf(&menuX, "DRONE", 5, droneChildren, 2);

    const char * envTypeNames[] = {"ADSR", "AR"};
    createMenuLeaf(&menuX, "TYPE", "ENVELOPE TYPE", 0, 1, &settings.envelopeType, envTypeNames, envTypeNames);
    createMenuLeaf(&menuX, "ATTACK", "ENVELOPE ATTACK TIME", 0, 1000, &settings.attack, NULL, NULL);
    createMenuLeaf(&menuX, "DECAY", "ENVELOPE DECAY TIME", 0, 1000, &settings.decay, NULL, NULL);
    createMenuLeaf(&menuX, "SUST", "ENVELOPE SUSTAIN", 0, 100, &settings.sustain, NULL, NULL);
    createMenuLeaf(&menuX, "RELE", "ENVELOPE RELEASE TIME", 0, 1000, &settings.release, NULL, NULL);
    const char * envelopeChildren[] = {"TYPE", "ATTACK", "DECAY", "SUST", "RELE"};
    createMenuNonLeaf(&menuX, "ENVELOPE", 4, envelopeChildren, 5);

    createMenuLeaf(&menuX, "CHAN", "MIDI CHANNEL", 1, 16, &settings.channel, NULL, NULL);
    createMenuLeaf(&menuX, "BRIGHT", "DISPLAY BRIGHTNESS", 0, 1, &settings.brightness, highLowNames, highLowNames);
    createMenuAction(&menuX, "CLEARDATA", 5);
    const char * systemChildren[] = {"CHAN", "BRIGHT", "CLEARDATA"};
    createMenuNonLeaf(&menuX, "SYSTEM", 6, systemChildren, 3);

    createMenuAction(&menuX, "ACTION", 6);

    const char * menuChildren[] = {"ENVELOPE", "DRONE", "ACTION", "SYSTEM"};
    createMenuRoot(&menuX, menuChildren, 4);
}

char readInput() {
    printf("\nEnter commands ([a] left, [d] right, [w] into, [s] back, [q] quit): ");
    char command = 0;
    while (command < 'a' || command > 'z') {
        command = getchar();
    }
    return command;
}

int loop() {
    renderMenu(&menuX, lineTop, lineBottom, 64, true);

    printf("------\n");
    printf("%s\n", lineTop);
    printf("%s\n", lineBottom);

    char command = readInput();

    if (command == 'a') {
        goLeftMenu(&menuX);
    } else if (command == 'd') {
        goRightMenu(&menuX);
    } else if (command == 'w') {
        goIntoMenu(&menuX);
    } else if (command == 's') {
        goBackMenu(&menuX);
    } else if (command == 'q') {
        return 0;
    }

    return 1;
}

void setup() {
    setupMenu();
}

int main() {
    setup();
    while (loop()) {}
    return 0;
}