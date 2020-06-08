
#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

#include "io.h"
#include "scene.h"
#include "logic.h"
#include "variables.h"
#include "helper.h"

enum e_Direction direction = dirNone;
GLboolean showFullscreen = GL_FALSE;
double cooldown = 0.0f;

// TODO: Neuschreiben
void changeCameraOrientation(GLfloat deltaRadius, GLfloat deltaPolar, GLfloat deltaAzimuth) {
    getGame()->camera.radius += deltaRadius;
    getGame()->camera.polarAngle += deltaPolar;
    getGame()->camera.azimuthAngle += deltaAzimuth;

    if (getGame()->camera.radius < CAMERA_RADIUS_MIN) {
        getGame()->camera.radius = CAMERA_RADIUS_MIN;
    }

    if (getGame()->camera.radius > CAMERA_RADIUS_MAX) {
        getGame()->camera.radius = CAMERA_RADIUS_MAX;
    }

    if (getGame()->camera.polarAngle < 0.0f) {
        getGame()->camera.polarAngle += 360.0f;
    }

    if (getGame()->camera.polarAngle > 360.0f) {
        getGame()->camera.polarAngle -= 360.0f;
    }

    if (getGame()->camera.azimuthAngle < 5.0f) {
        getGame()->camera.azimuthAngle = 5.0f;
    }

    if (getGame()->camera.azimuthAngle > 90.0f) {
        getGame()->camera.azimuthAngle = 90.0f;
    }
}


// TODO: Neuschreiben
void handleMouseEvent(int x, int y, CGMouseEventType eventType,
                      int button, int buttonState) {
    /* aktueller Status der linken Maustaste */
    static int leftMouseButtonState = GLUT_UP;

    /* Position der Maus beim letzten Aendern der Kamera */
    static int oldMousePos[2] = {0, 0};

    /* Veraenderung der Kameraausrichtung */
    float radius = 0.0f;
    float polar = 0.0f;
    float azimuth = 0.0f;

    switch (eventType) {
        case mouseButton:
            switch (button) {
                case GLUT_LEFT_BUTTON:
                    leftMouseButtonState = buttonState;
                    oldMousePos[0] = x;
                    oldMousePos[1] = y;
                    break;

                case 3: /* Hoch-Scrollen */
                    if (buttonState == GLUT_UP) {
                        radius -= SCROLL_SPEED;
                    }
                    break;

                case 4: /* Runter-Scrollen */
                    if (buttonState == GLUT_UP) {
                        radius += SCROLL_SPEED;
                    }
                    break;
            }
            break;

        case mouseMotion:
            if (leftMouseButtonState == GLUT_DOWN) {
                int deltaX = oldMousePos[0] - x;
                int deltaY = oldMousePos[1] - y;

                polar += ((float) deltaX) * POLAR_FACTOR;
                azimuth += ((float) deltaY) * AZIMUTH_FACTOR;

                oldMousePos[0] = x;
                oldMousePos[1] = y;
            }
            break;
        default:
            break;
    }

    changeCameraOrientation(radius, polar, azimuth);
}

/**
 * Setzen der Projektionsmatrix.
 * Setzt die Projektionsmatrix unter Berücksichtigung des Seitenverhaeltnisses
 * des Anzeigefensters, sodass das Seitenverhaeltnisse der Szene unveraendert
 * bleibt und gleichzeitig entweder in x- oder y-Richtung der Bereich von -1
 * bis +1 zu sehen ist.
 * @param aspect Seitenverhaeltnis des Anzeigefensters (In).
 */
void
setProjection(GLdouble aspect) {

    /* Nachfolgende Operationen beeinflussen Projektionsmatrix */
    glMatrixMode(GL_PROJECTION);

    /* Matrix zuruecksetzen - Einheitsmatrix laden */
    glLoadIdentity();

    /* perspektivische Projektion */
    gluPerspective(15.0,         /* Oeffnungswinkel */
                   aspect,       /* Seitenverhaeltnis */
                   0.1,          /* nahe Clipping-Ebene */
                   40.0 /* ferne Clipping-Ebene */ );
}

/**
 * Timer-Callback.
 * Initiiert Berechnung der aktuellen Position und anschliessendes Neuzeichnen,
 * setzt sich selbst erneut als Timer-Callback.
 * @param lastCallTime Zeitpunkt, zu dem die Funktion als Timer-Funktion
 *   registriert wurde (In).
 */
static void
cbTimer(int lastCallTime) {
    /* Seit dem Programmstart vergangene Zeit in Millisekunden */
    int thisCallTime = glutGet(GLUT_ELAPSED_TIME);

    if (cooldown < 0) {
        setPlayerMovement(direction);
        cooldown = COOLDOWN_TIME;
    } else {
        cooldown -= (double) (thisCallTime - lastCallTime) / 1000.0f;
    }

    /* Wieder als Timer-Funktion registrieren */
    glutTimerFunc(1000 / TIMER_CALLS_PS, cbTimer, thisCallTime);

    /* Neuzeichnen anstossen */
    glutPostRedisplay();
}

/**
 * Wird aufgerufen, um die Zeit fuer das Spiel zu zeichnen
 */
static void
decreaseTimer() {
    /* Seit dem Programmstart vergangene Zeit in Millisekunden */
    int thisCallTime = glutGet(GLUT_ELAPSED_TIME);

    /* Wieder als Timer-Funktion registrieren */
    glutTimerFunc(1000, decreaseTimer, thisCallTime);

    if (getGame()->gameStatus == GAME_RUNNING) {
        decreaseTime();
    }

    /* Neuzeichnen anstossen */
    glutPostRedisplay();
}

/**
 * Setzt einen Viewport fuer 2-dimensionale Darstellung.
 *
 * @param x, y Position des Viewports im Fenster - (0, 0) ist die untere linke Ecke
 * @param width, height Breite und Hoehe des Viewports
 */
static void set2DViewport(GLint x, GLint y, GLint width, GLint height) {
    /* Seitenverhaeltnis bestimmen */
    double aspect = (double) width / height;

    /* Folge Operationen beeinflussen die Projektionsmatrix */
    glMatrixMode(GL_PROJECTION);

    /* Einheitsmatrix laden */
    glLoadIdentity();

    /* Viewport-Position und -Ausdehnung bestimmen */
    glViewport(x, y, width, height);

    /* Das Koordinatensystem bleibt immer quadratisch */
    if (aspect <= 1) {
        gluOrtho2D(-1, 1,                     /* left, right */
                   -1 / aspect, 1 / aspect); /* bottom, top */
    } else {
        gluOrtho2D(-1 * aspect, 1 * aspect, /* left, right */
                   -1, 1);                    /* bottom, top */
    }

    /* Folge Operationen beeinflussen die Modelviewmatrix */
    glMatrixMode(GL_MODELVIEW);

    /* Einheitsmatrix laden */
    glLoadIdentity();
}

/**
 * Setzt einen Viewport fuer 3-dimensionale Darstellung
 * mit perspektivischer Projektion und legt eine Kamera fest.
 *
 * @param x, y Position des Viewports im Fenster - (0, 0) ist die untere linke Ecke
 * @param width, height Breite und Hoehe des Viewports
 */
static void set3DViewport(GLint x, GLint y, GLint width, GLint height) {
    /* Seitenverhaeltnis bestimmen */
    double aspect = (double) width / height;

    /* Folge Operationen beeinflussen die Projektionsmatrix */
    glMatrixMode(GL_PROJECTION);

    /* Einheitsmatrix laden */
    glLoadIdentity();

    /* Viewport-Position und -Ausdehnung bestimmen */
    glViewport(x, y, width, height);

    /* Perspektivische Darstellung */
    gluPerspective(70,        /* Oeffnungswinkel */
                   aspect,  /* Seitenverhaeltnis */
                   0.05,    /* nahe Clipping-Ebene */
                   100);    /* ferne Clipping-Ebene */

    /* Folge Operationen beeinflussen die Modelviewmatrix */
    glMatrixMode(GL_MODELVIEW);

    /* Einheitsmatrix laden */
    glLoadIdentity();
}

/**
 * Zeichen-Callback.
 * Loescht die Buffer, ruft das Zeichnen der Szene auf und tauscht den Front-
 * und Backbuffer.
 */
static void
cbDisplay(void) {
    /* Fensterdimensionen auslesen */
    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);

    /* Buffer zuruecksetzen */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* 3D Ansicht */
    set3DViewport(0, 0, width, height);
    drawScene(GL_TRUE);

    /* 2D Minimap */
    set2DViewport(width / 3 * 1.9, height / 2.2, width / 3, height / 2);
    drawScene(GL_FALSE);

    /* Objekt anzeigen */
    glutSwapBuffers();
}

/**
 * Callback fuer Aenderungen der Fenstergroesse.
 * Initiiert Anpassung der Projektionsmatrix an veränderte Fenstergroesse.
 * @param w Fensterbreite (In).
 * @param h Fensterhoehe (In).
 */
static void
cbReshape(int w, int h) {
    /* Das ganze Fenster ist GL-Anzeigebereich */
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);

    /* Anpassen der Projektionsmatrix an das Seitenverhältnis des Fensters */
    setProjection((GLdouble) w / (GLdouble) h);
}

void switchGameStatus(Gamestatus status) {
    if (getGame()->gameStatus == GAME_RUNNING) {
        getGame()->gameStatus = status;
    } else if (getGame()->gameStatus == status) {
        getGame()->gameStatus = GAME_RUNNING;
    }
}

// TODO: Neuschreiben
/**
 * Mouse-Button-Callback.
 * @param button Taste, die den Callback ausgeloest hat.
 * @param state Status der Taste, die den Callback ausgeloest hat.
 * @param x X-Position des Mauszeigers beim Ausloesen des Callbacks.
 * @param y Y-Position des Mauszeigers beim Ausloesen des Callbacks.
 */
static void cbMouseButton(int button, int state, int x, int y) {
    handleMouseEvent(x, y, mouseButton, button, state);
}
// TODO: Neuschreiben
/**
 * Mouse-Motion-Callback.
 * @param x X-Position des Mauszeigers.
 * @param y Y-Position des Mauszeigers.
 */
static void cbMouseMotion(int x, int y) {
    handleMouseEvent(x, y, mouseMotion, 0, 0);
}
// TODO: Neuschreiben
/**
 * Mouse-Passive-Motion-Callback.
 * @param x X-Position des Mauszeigers.
 * @param y Y-Position des Mauszeigers.
 */
static void
cbMousePassiveMotion(int x, int y) {
    handleMouseEvent(x, y, mousePassiveMotion, 0, 0);
}

/**
 * Verarbeitung eines Tasturereignisses.
 * Pfeiltasten steuern die Position des angezeigten Rechtecks.
 * F1-Taste (de-)aktiviert Wireframemodus.
 * f-Taste schaltet zwischen Fenster und Vollbilddarstellung um.
 * ESC-Taste und q, Q beenden das Programm.
 * @param key Taste, die das Ereignis ausgeloest hat. (ASCII-Wert oder WERT des
 *        GLUT_KEY_<SPECIAL>.
 * @param status Status der Taste, GL_TRUE=gedrueckt, GL_FALSE=losgelassen.
 * @param isSpecialKey ist die Taste eine Spezialtaste?
 * @param x x-Position des Mauszeigers zum Zeitpunkt der Ereignisausloesung.
 * @param y y-Position des Mauszeigers zum Zeitpunkt der Ereignisausloesung.
 */
static void
handleKeyboardEvent(int key, int status, GLboolean isSpecialKey, int x,
                    int y) {
#define ESC 27

    /* Taste gedrueckt */
    if (status == GLUT_DOWN) {

        Gamestatus s = getGame()->gameStatus;
        if (s == GAME_WON) {
            // Naechstes Level
            if (getGame()->levelId < NUMBER_OF_LEVEL) {
                initLevel(++getGame()->levelId);
            } else {
                initLevel(1);
            }
        } else if (s == GAME_LOST) {
            initLevel(getGame()->levelId);
        }

        /* Spezialtaste gedrueckt */
        if (isSpecialKey) {
            switch (key) {
                /* Bewegung des Rechtecks in entsprechende Richtung beenden */
                case GLUT_KEY_LEFT:
                    direction = dirLeft;
                    break;
                case GLUT_KEY_RIGHT:
                    direction = dirRight;
                    break;
                case GLUT_KEY_UP:
                    direction = dirUp;
                    break;
                case GLUT_KEY_DOWN:
                    direction = dirDown;
                    break;
                case GLUT_KEY_F1:
                    toggleWireframe();
                    break;
                case GLUT_KEY_F2:
                    toggleFullscreen();
                    break;
            }
        }
            /* normale Taste gedrueckt */
        else {
            switch (key) {
                case 'h':
                case 'H':
                    switchGameStatus(GAME_HELP);
                    break;

                case 'p':
                case 'P':
                    switchGameStatus(GAME_PAUSED);
                    break;

                case '1':
                    initLevel(1);
                    break;

                case '2':
                    initLevel(2);
                    break;

                case '3':
                    initLevel(3);
                    break;

                case '4':
                    initLevel(4);
                    break;

                case 'r':
                case 'R':
                    initLevel(getGame()->levelId);
                    break;

                case 'a':
                case 'A':
                    setPlayerMovement(dirLeft);
                    break;
                case 'd':
                case 'D':
                    setPlayerMovement(dirRight);
                    break;
                case 'w':
                case 'W':
                    setPlayerMovement(dirUp);
                    break;

                case 's':
                case 'S':
                    setPlayerMovement(dirDown);
                    break;
                    /* Programm beenden */
                case 'q':
                case 'Q':
                case ESC:
                    exit(0);
            }
        }
    } else if (status == GLUT_UP) {

        /* Spezialtaste gedrueckt */
        if (isSpecialKey) {
            switch (key) {
                /* Bewegung des Rechtecks in entsprechende Richtung beenden */
                case GLUT_KEY_LEFT:
                    direction = dirNone;
                    break;
                case GLUT_KEY_RIGHT:
                    direction = dirNone;
                    break;
                case GLUT_KEY_UP:
                    direction = dirNone;
                    break;
                case GLUT_KEY_DOWN:
                    direction = dirNone;
                    break;
            }
        }
    }
}

/**
 * Callback fuer Tastendruck.
 * Ruft Ereignisbehandlung fuer Tastaturereignis auf.
 * @param key betroffene Taste (In).
 * @param x x-Position der Maus zur Zeit des Tastendrucks (In).
 * @param y y-Position der Maus zur Zeit des Tastendrucks (In).
 */
static void
cbKeyboard(unsigned char key, int x, int y) {
    handleKeyboardEvent(key, GLUT_DOWN, GL_FALSE, x, y);
}

/**
 * Callback fuer Tastenloslassen.
 * Ruft Ereignisbehandlung fuer Tastaturereignis auf.
 * @param key betroffene Taste (In).
 * @param x x-Position der Maus zur Zeit des Loslassens (In).
 * @param y y-Position der Maus zur Zeit des Loslassens (In).
 */
static void
cbKeyboardUp(unsigned char key, int x, int y) {
    handleKeyboardEvent(key, GLUT_UP, GL_FALSE, x, y);
}


/**
 * Callback fuer Druck auf Spezialtasten.
 * Ruft Ereignisbehandlung fuer Tastaturereignis auf.
 * @param key betroffene Taste (In).
 * @param x x-Position der Maus zur Zeit des Tastendrucks (In).
 * @param y y-Position der Maus zur Zeit des Tastendrucks (In).
 */
static void
cbSpecial(int key, int x, int y) {
    handleKeyboardEvent(key, GLUT_DOWN, GL_TRUE, x, y);
}

/**
 * Callback fuer Loslassen von Spezialtasten.
 * Ruft Ereignisbehandlung fuer Tastaturereignis auf.
 * @param key betroffene Taste (In).
 * @param x x-Position der Maus zur Zeit des Loslassens (In).
 * @param y y-Position der Maus zur Zeit des Loslassens (In).
 */
static void
cbSpecialUp(int key, int x, int y) {
    handleKeyboardEvent(key, GLUT_UP, GL_TRUE, x, y);
}

/**
 * Registrierung der GLUT-Callback-Routinen.
 */
void registerCallbacks(void) {

    /* Tasten-Druck-Callback - wird ausgefuehrt, wenn eine Taste gedrueckt wird */
    glutKeyboardFunc(cbKeyboard);

    /* Tasten-Loslass-Callback - wird ausgefuehrt, wenn eine Taste losgelassen wird */
    glutKeyboardUpFunc(cbKeyboardUp);

    /* Spezialtasten-Druck-Callback - wird ausgefuehrt, wenn Spezialtaste
     * (F1 - F12, Links, Rechts, Oben, Unten, Bild-Auf, Bild-Ab, Pos1, Ende oder Einfuegen)
     * gedrueckt wird
     * */
    glutSpecialFunc(cbSpecial);

    /* Mouse-Button-Callback (wird ausgefuehrt, wenn eine Maustaste
 * gedrueckt oder losgelassen wird) */
    glutMouseFunc(cbMouseButton);

    /* Mouse-Motion-Callback (wird ausgefuehrt, wenn die Maus bewegt wird,
     * waehrend eine Maustaste gedrueckt wird) */
    glutMotionFunc(cbMouseMotion);

    /* Mouse-Motion-Callback (wird ausgefuehrt, wenn die Maus bewegt wird,
     * waehrend keine Maustaste gedrueckt wird) */
    glutPassiveMotionFunc(cbMousePassiveMotion);


    /* Spezialtasten-Loslass-Callback - wird ausgefuehrt, wenn eine Spezialtaste losgelassen wird */
    glutSpecialUpFunc(cbSpecialUp);

    /* Automat. Tastendruckwiederholung ignorieren */
    glutIgnoreKeyRepeat(1);

    /* Timer-Callback - wird einmalig nach msescs Millisekunden ausgefuehrt */
    glutTimerFunc(1000 / TIMER_CALLS_PS,
                  cbTimer,
                  glutGet(GLUT_ELAPSED_TIME));

    glutTimerFunc(1000, decreaseTimer, glutGet(GLUT_ELAPSED_TIME));

    /* Reshape-Callback - wird ausgefuehrt, wenn neu gezeichnet wird (z.B. nach
     * Erzeugen oder Groessenaenderungen des Fensters) */
    glutReshapeFunc(cbReshape);

    /* Display-Callback - wird an mehreren Stellen imlizit (z.B. im Anschluss an
 * Reshape-Callback) oder explizit (durch glutPostRedisplay) angestossen */
    glutDisplayFunc(cbDisplay);
}

/**
 * Initialisiert das Programm (inkl. I/O und OpenGL) und startet die
 * Ereignisbehandlung.
 * @param title Beschriftung des Fensters
 * @param width Breite des Fensters
 * @param height Hoehe des Fensters
 * @return ID des erzeugten Fensters, 0 im Fehlerfall
 */
int
initAndStartIO(char *title, int width, int height) {
    int windowID;

    /* Kommandozeile immitieren */
    int argc = 1;
    char *argv = "cmd";

    /* Glut initialisieren */
    glutInit(&argc, &argv);

    /* Initialisieren des Fensters */
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);

    //Tiefentest (in der init()) aktivieren
    glEnable(GL_DEPTH_TEST);

    // Auf dem aktuellen Bildschirm anzeigen
    glutInitWindowPosition(glutGet(GLUT_SCREEN_WIDTH) / 2, glutGet(GLUT_SCREEN_HEIGHT) / 2);

    /* Fenster erzeugen */
    windowID = glutCreateWindow(title);

    if (windowID) {
        if (initScene()) {
            registerCallbacks();
            glutMainLoop();
        } else {
            glutDestroyWindow(windowID);
            windowID = 0;
        }
    }

    return windowID;
}

enum e_Direction getDirection() {
    return direction;
}