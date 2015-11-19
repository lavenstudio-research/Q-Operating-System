#include "kbDetect.h"

const int BUFSIZE = 256;
const char chars[256] =    { 
     0 ,  0 , '1', '2', '3', '4', '5', '6', '7', '8', 
    '9', '0', '-', '=',  0 ,'\t', 'q', 'w', 'e', 'r', 
    't', 'y', 'u', 'i', 'o', 'p', '[', ']',  0 ,  0 , 
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
   '\'', '`',  0 ,'\\', 'z', 'x', 'c', 'v', 'b', 'n', 
    'm', ',', '.', '/',  0 , '*',  0 , ' ',  0 ,
};
const char charsShift[256] =    { 
     0 ,  0 , '!', '@', '#', '$', '%', '^', '&', '*', 
    '(', ')', '_', '+',  0 ,'\t', 'Q', 'W', 'E', 'R', 
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}',  0 ,  0 , 
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
   '\"', '~',  0 , '|', 'Z', 'X', 'C', 'V', 'B', 'N', 
    'M', '<', '>', '?',  0 , '*',  0 , ' ',  0 ,
};
const char charsCapsLock[256] =    { 
     0 ,  0 , '1', '2', '3', '4', '5', '6', '7', '8', 
    '9', '0', '-', '=',  0 ,'\t', 'Q', 'W', 'E', 'R', 
    'T', 'Y', 'U', 'I', 'O', 'P', '[', ']',  0 ,  0 , 
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';',
   '\'', '`',  0 ,'\\', 'Z', 'X', 'C', 'V', 'B', 'N', 
    'M', ',', '.', '/',  0 , '*',  0 , ' ',  0 ,
};

uint8 backspaceOne(uint8 i, string buffstr) {
    kprintch('\b', 0x0F, false);
    i--;
    char old = buffstr[i];
    if(old == '\t') {
        int delCount = i % 4;
        delCount = delCount == 0? 4 : delCount;
        while(delCount--) {
            buffstr[i] = 0;
            kprintch('\b', 0x0F, false);
            i--;
        }
        i++;
    } else buffstr[i] = 0;
    return i;
}

uint8 backspaceMul(uint8 i, string buffstr) {
    char old; // This is a place holder
    do {
        kprintch('\b', 0x0F, false);
        i--;
        old = buffstr[i];
        buffstr[i] = 0;
    } while ((old >= 97 && old <= 122) || (old >= 65 && old <= 90));
    return i;
}

uint8 pushCtrlChar(uint8 i, string buffstr, char caps) {
    printch('^', 0x0F);
    buffstr[i] = '^';
    i++;
    kprintch(caps, 0x0F, false);
    buffstr[i] = caps;
    return ++i;
}

int charKeyPressed(string buffstr, uint8 ch, int i) {
    int toPrint = 0xFF;
    bool shiftMask = lshift || rshift;
    /* Shift and Caps on should be lowercase */
    if(shiftMask && !capslock) {
        toPrint = charsShift[ch];
    } else if(capslock && !shiftMask) {
        toPrint = charsCapsLock[ch];
    } else {
        toPrint = chars[ch];
    }
    if(ctrl) {
        /* Ctrl key pushes an Uppercase */
        return pushCtrlChar(i, buffstr, charsCapsLock[ch]);
    } else if (alt) {
        /* Alt key pushes a lowercase */
        return pushCtrlChar(i, buffstr, chars[ch]);
    }
    buffstr[i] = toPrint;
    kprintch(toPrint, 0x0F, false);
    return ++i;
}

/*
 * TODO: bufSize should be used to check for unexpected behaviour
 */
void readStr(string buffstr, uint32 bufSize)
{
    uint8 i = 0;
    bool reading = true;
    while(reading)
    {
        //print(buffstr,0x0F);
    	//exit the writer program when the Ctrl-Z key is pressed
	    if (progexit && writing)
	    {
	        clearScreen();
	        updateCursor();
	        writing = false;
	        progexit = false;
	        print("Q-Kernel>  ", 0x08);
	    }

        if (deleteStopX > 0)
        {
	        if (cursorX < deleteStopX)
	        {
	            cursorX = deleteStopX;
	        }
        }

	    if (newCmd && typingCmd)
	    {
	        startCmdX = cursorX;
	        startCmdY = cursorY;
	        newCmd = 0;
	    }
	
	    //Detect keypress and return string of characters pressed to the buffstr char array
        if(inportb(0x64) & 0x1)                 
        {
            uint8 value = inportb(0x60);
            //print("test", 0x3F);
            bool handled = false;
            switch(value)
            { 
            case 29:        // Left Ctrl Down
                ctrl = true;   // Toggle On
                break;
            case 157:       // Left Ctrl Up
                ctrl = false;   // Toggle Off
                break;
            case 1:         // Esc (Ctrl-z)
                if (writing) {
                    progexit = true;
                    reading = false;
                } else {
                    i = pushCtrlChar(i, buffstr, 'Z');
                }
                break;
            case 14:                // Backspace
                if (lshift || rshift) { // On of the shifts are activated
                    // Delete until space | non-word | different-cased-word
                    i = backspaceMul(i, buffstr);
                } else {
                    // No shift -> delete one char
                    i = backspaceOne(i, buffstr);
                }
                break;
            case 25:
                if (ctrl) {
                    if (writing)
                    {
                        cursorY = cursorY - 1;
                        cursorX = cursorX - 1;
                    }
                }
                break;
            case 28:				//This is the enter key, we need to add more functionality to it with Writer and other commands
                if (writing)
                {
                    printch('\n',0x0F);
                    buffstr[i] = '\n';
                    i++;
                }
                else
                {
                    reading = false;
                }
                break;
            case 30:
                if (ctrl) {
                    if (writing)
                    {
                        cursorX = 0;
                        handled = true;
                    } 
                    else 
                    {
                        moveCursorX(-cursorX + 11);
                        handled = true;
                    }
                }
                break;
            case 38:
                if (ctrl == 1) {
                    clearScreen();
                    // Returns command "skip" which does nothing
                    strcpy(buffstr, "skip");
                    handled = true;
                    return; 
                }
                break;
            case 42:        //Left shift 
                lshift = true;
                break;
            case 44:        // z or Ctrl-Z
                if (ctrl) {
                    if (writing) {
                        progexit = true;
                        reading = false;
                        handled = true;
                    }
                }
                break;
            case 48:
                if (ctrl) {
                    moveCursorX(-1);
                    handled = true;
                }
                break;
            case 49:
                if (ctrl) {
                    if (writing)
                    {
                        cursorY = cursorY + 1;
                        cursorX = cursorX - 1;
                        handled = true;
                    }
                } 
                
                break;
            case 54:            // Right shift on
                rshift = true;     // Toggle On
                break;
            case 56:            // Left/Right alt On
                alt = true;        // Toggle On
                break;
            case 58:            // Capslock down
                capslock = !capslock;
                break;
            case 72:                //Up arrow
                if (writing)
                {
                    cursorY = cursorY - 1;
                    cursorX = cursorX;
                }
                break;
            case 75:				//Left Arrow
                moveCursorX(-1);
                break;
            case 77:				//Right Arrow
                moveCursorX(1);
                break;
            case 80:				//Down Arrow
                if (writing)
                {
                    cursorY = cursorY + 1;
                    cursorX = cursorX;
                }
                break;
            case 170:           // Left shift released (http://wiki.osdev.org/PS2_Keyboard)
                lshift = false;
                break;
            case 182:           // Right shift released (http://wiki.osdev.org/PS2_Keyboard)
                rshift = false;     // Toggle Off
                break;
            case 184:           // Left/Right alt Off
                alt = false;        // Toggle Off
                break;
            }
            if(!handled && chars[value]) {
                i = charKeyPressed(buffstr, value, i);
            }
        }
    }
    buffstr[i] = 0;      
}
