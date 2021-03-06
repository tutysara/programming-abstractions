//
// buffer.cpp
//
// This file implements the EditorBuffer class using a linked list to 
// represent the buffer.
//
// It has been augmented to support operations (movement and deletion) on 
// word-boundaries along with copy and paste.
//
// --------------------------------------------------------------------------
// Attribution: "Programming Abstractions in C++" by Eric Roberts
// Chapter 13, Exercise 7 (list-based)
// Stanford University, Autumn Quarter 2012
// http://web.stanford.edu/class/archive/cs/cs106b/cs106b.1136/materials/CS106BX-Reader.pdf
//
// This file extends code from Figure 13-9.
// --------------------------------------------------------------------------
//
// Created by Glenn Streiff on 8/02/16.
// Copyright © 2016 Glenn Streiff. All rights reserved. (derivative work)
//

#include <iostream>
#include "buffer.h"
using namespace std;

static const char SPACE = ' ';
static const char NEWLINE = '\n';

// Implementation notes: Constructor
// ---------------------------------
// This function initializes an empty editor buffer represented as a linked
// list.  In this representation, the empty buffer contains a "dummy" cell whose
// ch field is never used.  The constructor must allocate this dummy cell and
// set the internal pointers correctly.

EditorBuffer::EditorBuffer() {
    start = cursor = new Cell;
    start->link = NULL;
    copyString = "";
}

// Implementation notes: Constructor
// ---------------------------------
// The destructor must delete every cell in the buffer.  Note that the loop
// structure is not exactly the standard for-loop pattern for processing every
// cell within a linked list.  The complication that forces this change is that
// the body of the loop can't free the current cell and later have the for
// loop use the link field of that cell to move to the next one.  to avoid this
// problem, this implementation copies the link pointer before calling delete.

EditorBuffer::~EditorBuffer() {
    Cell *cp = start;
    while (cp != NULL) {
        Cell *next = cp->link;
        delete cp;
        cp = next;
    }
}

// Implementation notes: moveCursor methods
// ----------------------------------------
// The six methods that move the cursor have different time complexities
// because the structure of a linked list is asymmetrical with respect to
// moving backward and forward.  The moveForward and moveCursorToStart
// methods operate in constant time.  By constrast, the moveCursorBackward
// and moveCursorToEnd methods require a loop that runs in linear time.

void EditorBuffer::moveCursorForward() {
    if (cursor->link != NULL) {
        cursor = cursor->link;
    }
}

void EditorBuffer::moveCursorForwardWord() {
    
    // Normalize pre-move position.
    //
    // Move forward to the nearest beginning-of-word character by advancing
    // through any whitespace immediately to the right of the cursor.
    
    while (cursor->link != NULL &&
           ((cursor->link->ch == SPACE) ||
            (cursor->link->ch == NEWLINE))) {
        moveCursorForward();
    }
    
    // Advance through the word to the immediate right of the cursor.
    //
    // Words are space-delimited or newline-delimited so stop advancing
    // if we encounter either of those.
    
    while (cursor->link != NULL &&
           cursor->link->ch != SPACE &&
           cursor->link->ch != NEWLINE) {
        moveCursorForward();
    }
}

void EditorBuffer::moveCursorBackward() {
    Cell *cp = start;
    if (cursor != start) {
        while (cp->link != cursor) {
            cp = cp->link;
        }
        cursor = cp;
    }
}

void EditorBuffer::moveCursorBackwardWord() {
    
    // Normalize pre-backup position.
    //
    // Backup to the nearest end-of-word character by scooting back
    // through any whitespace immediately to the left of the cursor.
    
    while (cursor != NULL &&
           (cursor->ch == SPACE || cursor->ch == NEWLINE)) {
               moveCursorBackward();
    }
    
    // Backup through the word to the immediate left of the cursor.
    //
    // Words are space-delimited or newline-delimited so stop backing up
    // if we encounter either of those.
    
    while (cursor != NULL && cursor->ch != SPACE && cursor->ch != NEWLINE) {
        moveCursorBackward();
    }
}

void EditorBuffer::moveCursorToStart() {
    cursor = start;
}

void EditorBuffer::moveCursorToEnd() {
    while (cursor->link != NULL) {
        cursor = cursor->link;
    }
}

// Implementation notes: insertCharacter
// -------------------------------------
// The steps required to insert a new character are:
//
//  1. Allocate a new cell and put the new character in it.
//  2. Copy the pointer to the remainder of the list into the link.
//  3. Update the link in the current cell to point to the new cell.
//  4. Move the cursor forward over the inserted character.

void EditorBuffer::insertCharacter(char ch) {
    Cell *cp = new Cell;
    cp->ch = ch;
    cp->link = cursor->link;
    cursor->link = cp;
    cursor = cp;
}

// Implementation notes: insertString
// ----------------------------------
// Extend the notion of insertion to apply to a string of characters.

void EditorBuffer::insertString(const std::string str) {
    for (int i = 0; i < str.length(); i++) {
        insertCharacter(str[i]);
    }
}

// Method: copy, copyWords, paste
// Usage: copy(nChars);
//        copyWords(nWords);
//        paste();
// -------------------------
// Copies the next n-characters following the cursor to a private
// buffer in anticipation of a subsequent paste command.

void EditorBuffer::copy(int nChars) {
    copyString = getText(cursor).substr(0, nChars);
}

void EditorBuffer::copyWords(int nWords) {
    string charString = getText(cursor);
    int i = 0;
    
    while (nWords > 0) {
        for (; i < charString.length(); i++) {
            if (charString[i] == SPACE || charString[i] == NEWLINE) {
                continue;   // Scoot past any prefixed whitespace before
                            // counting next word.
            }
            break;
        }
        for (; i < charString.length(); i++) {
            // Check for next end-of-word delimiter.
            if (charString[i] == SPACE || charString[i] == NEWLINE) {
                --nWords;
                break;
            }
        }
        if (i == charString.length()) {
            // We've parsed the whole string so force while to exit
            // even if we didn't reach the target word count.
            nWords = 0;
        }
    }
    copyString = charString.substr(0, i);
}

void EditorBuffer::paste() {
    insertString(copyString);
}

// Implementation notes: deleteCharacter
// -------------------------------------
// The steps necesarry to delete the character after the cursor are:
//
// 1. Remove the current cell by pointing to its successor.
// 2. Free the cell to reclaim the memory.

void EditorBuffer::deleteCharacter() {
    if (cursor->link != NULL) {
        Cell *oldCell = cursor->link;
        cursor->link = cursor->link->link;
        delete oldCell;
    }
}

// Implementation notes: deleteWord
// --------------------------------
// Deletes all the characters from the cursor position through to the end
// of the word just to the right of the cursor.  If the cursor is within the
// whitespace preceding a word, that is deleted first, followed by the word
// itself.

void EditorBuffer::deleteWord() {
    
    // Delete any preceding whitespace immediately to the right of the cursor.
    
    while (cursor->link != NULL &&
          (cursor->link->ch == SPACE || cursor->link->ch == NEWLINE)) {
        deleteCharacter();
    }
    
    // Delete the word characters until we hit an end-of-word delimiter
    // such as a space or a newline.
    
    while (cursor->link != NULL &&
           cursor->link->ch != SPACE &&
           cursor->link->ch != NEWLINE) {
        deleteCharacter();
    }
}

// Implementation notes: getText and getCursor
// -------------------------------------------
// The getText method uses the standard linked-list pattern to loop through
// the cells in the linked list.  The getCursor method must count the
// characters in the list unit it reaches the cursor.

string EditorBuffer::getText() const {
    string str = "";
    for (Cell *cp = start->link; cp != NULL; cp = cp->link) {
        str += cp->ch;
    }
    return str;
}

string EditorBuffer::getText(const Cell *startCell) const {
    string str = "";
    for (Cell *cp = startCell->link; cp != NULL; cp = cp->link) {
        str += cp->ch;
    }
    return str;
}
int EditorBuffer::getCursor() const {
    int nChars = 0;
    for (Cell *cp = start; cp != cursor; cp = cp->link) {
        nChars++;
    }
    return nChars;
}