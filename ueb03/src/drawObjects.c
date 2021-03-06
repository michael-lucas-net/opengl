#include <math.h>
#include "debug.h"
#include "types.h"
#include "logic.h"
#include "helper.h"

// Werte fuer Portal-Animation
float shrinkVal = 0;
GLboolean isIncreasing = GL_TRUE;
GLfloat houseColors[3] = {1.0f, 0.0f, 1.0f};

/**
 * Setzt das Materiallicht fuer die gezeichnetetn Objekte
 * @param r - rot
 * @param g - gruen
 * @param b - blau
 */
void setMaterialLightning(float r, float g, float b) {
    /* Verringert die Saetting der Farben, sodass nur noch x% angzeigt werden */
    float multiplier = 0.15f;

    float matDiffuse[] = {r, g, b, 1};
    float matAmbient[] = {r * multiplier, g * multiplier, b * multiplier, 1.0f};
    float matSpecular[] = {r, g, b, 1.0f};
    float matShininess = 20;

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &matShininess);
}

/**
 * Aendert die Farbe des Hauses
 * @param isGreenHouse - wenn True, dann Farbe Gruen
 */
void changeColor(GLboolean isGreenHouse) {
    if (isGreenHouse) {
        houseColors[0] = 0.196f;
        houseColors[1] = 0.804f;
        houseColors[2] = 0.196f;
    } else {
        houseColors[0] = 1.0f;
        houseColors[1] = 0.0f;
        houseColors[2] = 1.0f;
    }
}

/**
 * Zeichnet ein 3D-Viereck
 */
static void drawSquare() {

    int x, y;
    float w = 0.5f;

    /* Normalen zeichnen */
    if (getGame()->settings.showNormals) {
        glBegin(GL_LINES);
        {
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.3f);
        }
        glEnd();
    }

    /* Viele kleine Squares erstellen */
    for (y = 0; y < SQUARE_REFINEMENT_COUNT; y++) {
        glBegin(GL_QUAD_STRIP);
        {
            glNormal3f(0.0f, 0.0f, 1.0f);
            for (x = 0; x <= SQUARE_REFINEMENT_COUNT; x++) {
                glVertex3f(-w + (float) x / (SQUARE_REFINEMENT_COUNT),
                           w - (float) y / (SQUARE_REFINEMENT_COUNT),
                           0.0f);

                glVertex3f(-w + (float) x / (SQUARE_REFINEMENT_COUNT),
                           w - (float) (y + 1) / (SQUARE_REFINEMENT_COUNT),
                           0.0f);
            }
        }
        glEnd();
    }
}

/**
 * Zeichnet einen Zylinder
 */
static void drawCylinder() {
    GLUquadricObj *quadratic;
    quadratic = gluNewQuadric();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    gluCylinder(quadratic, 0.2f, 0.2f, 1.0f, 32, 32);
}

/**
 * Zeichnet ggf. die Normalen an der angegebenen Stelle
 * @param x - Wert
 * @param y - Wert
 * @param z - Wert
 */
void drawNormals(float x, float y, float z) {
    if (getGame()->settings.showNormals) {
        glBegin(GL_LINES);
        {
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

/**
 * Zeichnet eine Pyramide
 */
static void drawPyramid() {
    drawNormals(0.5f, 0, 1.0f);
    drawNormals(1.0f, 0.0f, 0.5f);
    drawNormals(0.0f, 1, -1);
    drawNormals(-1, 0.5f, 0.5f);

    glBegin(GL_TRIANGLES);

    // Vorne
    glNormal3f(0.5f, 0.1f, 1.0);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);


    // Rechts
    glNormal3f(1.0f, 0.0f, 0.5f);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Hinten
    glNormal3f(0.0, 1.0, -1.0);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, -0.5f);

    // Links
    glNormal3f(-1.0, 0.5, 0.5);
    glVertex3f(0.0f, 0.5f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glEnd();
}

/**
 * Zeichnet einen Thyreophora
 */
static void drawTetrahedron() {
    if (getGame()->settings.showNormals) {
        drawNormals(0, 1, 1);
        drawNormals(1, 1, -1);
        drawNormals(-1, 1, -1);
    }

    glBegin(GL_TRIANGLE_STRIP);
    glNormal3f(0, 1, 1);

    glVertex3f(0, 1, 0);
    glVertex3f(-0.5, 0, 0.5);
    glVertex3f(0.5, 0, 0.5);

    glNormal3f(1, 1, -1);

    glVertex3f(0, 0, -0.7);
    glVertex3f(0, 1, 0);
    glVertex3f(-0.5, 0, 0.5);

    glNormal3f(-1, 1, -1);

    glEnd();
}

/**
 * Zeichnet einen Kreis (fuer den Ball)
 */
static void drawCircle() {

    float radius = 1.0f;
    GLUquadric *quad = gluNewQuadric();
    gluDisk(quad, 0, radius, 32, 1);

    gluDeleteQuadric(quad);
}

/**
 * Zeichnet gerade Striche (z.B. fuer die Mauern)
 * @param width  - Breite (wird ignoriert bei isHorizontal == true)
 * @param heigth - Hoehe (wird ignoriert bei isHorizontal == true)
 * @param isHorizontal - True, wenn horizontal gezeichnet werden soll
 */
void drawDash(float width, float heigth, GLboolean isHorizontal) {

    float w = width;
    float h = heigth;

    if (isHorizontal) {
        w = w < 0.00001f ? 1 : w;
        h = h < 0.00001f ? FUGUE_WIDTH : h;
    }

    glPushMatrix();
    {
        glScalef(w, h, 1.0f);
        drawSquare();
    }
    glPopMatrix();
}

/**
 * Zeichnet die Mauer
 */
void drawWall() {

    glColor3f(0.412f, 0.412f, 0.412f);
    setMaterialLightning(0.412f, 0.412f, 0.412f);

    glPushMatrix();
    {
        drawSquare();

        // Fugenfarbe
        glColor3f(0.663f, 0.663f, 0.663f);
        setMaterialLightning(0.663f, 0.663f, 0.663f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, -FUGUE_HEIGHT * 2, 0.001f);
            // Horizontale Striche
            for (int i = 0; i < 3; i++) {
                glTranslatef(0.0f, FUGUE_HEIGHT, 0.0f);
                drawDash(0, 0, GL_TRUE);
            }
        }
        glPopMatrix();

        // Vertikal
        glPushMatrix();
        {
            glTranslatef(0.0f, -FUGUE_HEIGHT * 2.5f, 0.001);
            for (int i = 0; i < 4; i++) {

                glTranslatef(0.0, FUGUE_HEIGHT, 0);
                int max = i % 2 == 0 ? 3 : 2;

                glPushMatrix();
                {
                    glTranslatef(-FUGUE_HEIGHT * 2, 0, 0);
                    for (int j = 0; j < max; ++j) {
                        float xVal = FUGUE_HEIGHT;
                        if (max == 2) {
                            xVal = FUGUE_HEIGHT + 0.1f;
                        }
                        glTranslatef(xVal, 0, 0);

                        drawDash(FUGUE_WIDTH, FUGUE_HEIGHT, GL_FALSE);
                    }
                }
                glPopMatrix();
            }
        }
        glPopMatrix();
    }

    glPopMatrix();
}

/**
 * Zeichnet die Box
 */
void drawBox() {
    glColor3f(0.600f, 0.240f, 0.100f);
    setMaterialLightning(0.600f, 0.240f, 0.100f);

    float bottom = -0.5f;
    float offset = 0.15f;

    glPushMatrix();
    {
        drawSquare();

        // Fugenfarbe
        glColor3f(0.1f, 0.1f, 0.1f);
        setMaterialLightning(0.1f, 0.1f, 0.1f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, bottom + offset, 0.002f);

            // Horizontale Striche
            for (int i = 0; i < 2; i++) {
                drawDash(0, 0, GL_TRUE);
                glTranslatef(0.0f, -(bottom + offset) * 2, 0.0f);
            }
        }
        glPopMatrix();

        // Vertikal
        glPushMatrix();
        {
            glTranslatef(-0.4f, 0.0f, 0.001f);
            for (int i = 0; i < BOX_NUMBER_OF_COLS; i++) {
                drawDash(BOX_DASH_WIDTH, BOX_DASH_HEIGHT, GL_FALSE);
                glTranslatef(1.0f / (float) BOX_NUMBER_OF_COLS - (BOX_DASH_WIDTH / 2), 0.0, 0.0f);
            }
        }
        glPopMatrix();

    }
    glPopMatrix();
}

/**
 * Zeichnet den freien Block
 */
void drawFreeBlock() {
    glColor3f(0.663f, 0.663f, 0.663f);
    setMaterialLightning(0.663f, 0.663f, 0.663f);

    glPushMatrix();
    {
        glRotatef(-90, 1, 0, 0);
        glScalef(BLOCK_SIZE, BLOCK_SIZE, 1.0f);
        drawSquare();
    }

    glPopMatrix();
}

/**
 * Zeichnet die Tuer
 */
void drawDoor() {
    glColor3f(0.600f, 0.240f, 0.100f);
    setMaterialLightning(0.600f, 0.240f, 0.100f);

    glPushMatrix();
    {
        drawSquare();

        // Braun
        glColor3f(0.1f, 0.1f, 0.1f);
        setMaterialLightning(0.1f, 0.1f, 0.1f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, 0.0, 0.001f);
            drawDash(0, 0, GL_TRUE);
        }
        glPopMatrix();
    }
    glPopMatrix();
}

/**
 * Zeichnet ein Dreieckobjekt
 */
void drawTriangleOject() {
    drawFreeBlock();
    glColor3f(0.137f, 0.137f, 0.557f);
    setMaterialLightning(0.137f, 0.137f, 0.557f);

    glPushMatrix();
    {
        glScalef(0.12f, 0.12f, 0.12f);
        drawTetrahedron();
    }
    glPopMatrix();
}

/**
 * Zeichnet den blauen Kasten
 * (Ziel fuer die Dreiecke)
 */
void drawFinish() {
    glColor3f(0.137f, 0.137f, 0.557f);
    setMaterialLightning(0.137f, 0.137f, 0.557f);

    glPushMatrix();
    {
        glRotatef(-90, 1, 0, 0);
        glScalef(BLOCK_SIZE, BLOCK_SIZE, 1.0f);
        drawSquare();
    }

    glPopMatrix();
}

/**
 * Setzt transparente (Material) Farben
 * @param r - rot
 * @param g - gruen
 * @param b  - blaue
 * @param alpha - alpha :)
 */
void setTransparentColors(float r, float g, float b, float alpha) {
    float opacity = 0.1f;
    float diffuse[] = {r, g, b, alpha};
    float ambient[] = {
            r * opacity,
            g * opacity,
            b * opacity,
            alpha * opacity
    };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);

    glColor4fv(diffuse);
}

/**
 * Zeichnet einen Pfeil fuer den Tuerschalter
 */
void drawDoorSwitchArrow() {

    /* Moeglichkeit fuer Transparenz anschalten */
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    {
        setTransparentColors(0.2f, 0.4f, 0.8f, 0.5f);

        // 2x Tetraeder
        for (int i = 0; i < 2; ++i) {
            float posX = i == 0 ? 0.0f : 1.0f;
            int angle = i == 0 ? 90 : -90;

            glPushMatrix();
            {
                glScalef(0.1f, 0.1f, 0.1f);
                glTranslatef(posX, 0.0f, 0.0f);
                glRotatef(angle, 0, 0, 1);
                drawTetrahedron();
            }
            glPopMatrix();
        }

        // Zylinder
        glPushMatrix();
        {
            glScalef(0.1f, 0.1f, 0.1f);
            glTranslatef(0.0f, 0.0f, 0.0f);
            drawCylinder();
        }
        glPopMatrix();

    }
    glPopMatrix();

    /* Transparenz ausschalten */
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

static void
drawSphere(void) {
    /* Quadric erzuegen */
    GLUquadricObj *qobj = gluNewQuadric();
    if (qobj != 0) {

        /* Normalen fuer Quadrics berechnen lassen */
        gluQuadricNormals(qobj, GLU_SMOOTH);

        /* Kugel zeichen */
        gluSphere(qobj, 0.5, 20, 20);

        /* Loeschen des Quadrics nicht vergessen */
        gluDeleteQuadric(qobj);
    } else {
        CG_ERROR (("Could not create Quadric\n"));
    }
}

/**
 * Zeichnet den Tuerschalter
 */
void drawDoorSwitch() {
    drawFreeBlock();

    float scaleVal = 0.5f;
    glPushMatrix();
    {
        glScalef(scaleVal, scaleVal, scaleVal);
        glTranslatef(-0.1f, 0.06f, 0.05f);
        drawDoorSwitchArrow();
    }
    glPopMatrix();
}

/**
 * Zeichnet die Tür und die Fenster des Hauses
 */
void drawHouseFront() {
    glColor3f(houseColors[0], houseColors[1], houseColors[2]);
    setMaterialLightning(houseColors[0], houseColors[1], houseColors[2]);

    // Tür
    glPushMatrix();
    {
        glTranslatef(0, -0.34, 0.03f);
        glScalef(0.18f, 0.3f, 0.03f);
        drawSquare();
        glColor3f(1, 1, 1);
        setMaterialLightning(1, 1, 1);
    }
    glPopMatrix();

    // Fenster
    for (int i = 0; i < 2; ++i) {
        float x = i == 0 ? -0.3f : 0.3f;
        glPushMatrix();
        {
            glColor3f(0.529f, 0.808f, 0.922f);
            setMaterialLightning(0.529f, 0.808f, 0.922f);

            glTranslatef(x, 0.16, 0.003);
            glScalef(0.24f, 0.24f, 0.015f);

            drawSquare();
        }
        glPopMatrix();
    }
    glColor3f(1, 1, 1);
    setMaterialLightning(1, 1, 1);
}

/**
 * Zeichnen der einzelnen Seiten des Quaders
 * @param type Typ des fieldtypes
 * @param specialCase Sonderbehandlung, z.B. Tür an Hauswand
 */
void paintCube(const pushyFieldType *type, GLboolean specialCase) {

    if ((*type) == P_BOX) {
        drawBox();
    } else if ((*type) == P_WALL) {
        drawWall();
    } else if ((*type) == P_DOOR) {

        /* Tuer drehen, damit Schlitz an der richtigen Stelle ist */
        if (specialCase == 1) {
            glRotatef(90, 0, 0, 1);
        }

        drawDoor();
    } else if ((*type) == P_HOUSE) {

        if (specialCase == 1) {
            drawHouseFront();
        }
        drawSquare();
    } else {
        drawSquare();
    }
}

/**
 * Zeichnet einen Cube
 * @param type - Typ des Feldes (z.B. P_WALL)
 */
void
drawCube(pushyFieldType type) {

    if (type == P_DOOR) {
        drawFreeBlock();
    }

    glPushMatrix();
    {
        glTranslatef(0, 0.11f, 0);

        if (type == P_DOOR) {
            glScalef(0.1f, BLOCK_SIZE, BLOCK_SIZE);
        } else {
            glScalef(BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
        }

        /* Frontflaeche */
        glPushMatrix();
        {
            glTranslatef(0.0f, 0.0f, 0.5f);
            paintCube(&type, GL_FALSE);
        }
        glPopMatrix();

        /* rechte Seitenflaeche */
        glPushMatrix();
        {
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.5f);

            paintCube(&type, GL_TRUE);
        }
        glPopMatrix();

        /* Rueckseitenflaeche */
        glPushMatrix();
        {
            glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.5f);
            paintCube(&type, GL_FALSE);
        }
        glPopMatrix();

        /* linke Seitenflaeche */
        glPushMatrix();
        {
            glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.5f);
            paintCube(&type, GL_TRUE);
        }
        glPopMatrix();

        /* Deckelflaeche */
        glPushMatrix();
        {
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.5f);
            paintCube(&type, GL_FALSE);
        }
        glPopMatrix();

        /* Bodenflaeche */
        glPushMatrix();
        {
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.5f);
            paintCube(&type, GL_FALSE);
        }
        glPopMatrix();
    }
    glPopMatrix();
}

/**
 * Zeichnet die Portale
 */
void drawPortals() {
    drawFreeBlock();
    float size = 0.1f;
    float portalSize;
    float shrinkInterval = 15.0f;

    portalSize = 0 + shrinkVal;

    // Portal animieren
    isIncreasing
    ? (shrinkVal += 0.1f / shrinkInterval)
    : (shrinkVal -= 0.1f / shrinkInterval);

    // Wenn das Portal auf 0 ist, wird es vergroessert
    // sonst verkleinert
    if (portalSize <= 0) {
        isIncreasing = GL_TRUE;
    } else if (portalSize >= 1) {
        isIncreasing = GL_FALSE;
    }

    glPushMatrix();
    {
        glScalef(portalSize, 0.02f, portalSize);

        // 5 Farben, 5 Ringe
        for (int i = 0; i < 5; ++i) {
            float *colors = selectColor(i);
            glColor3f(colors[0], colors[1], colors[2]);
            setMaterialLightning(colors[0], colors[1], colors[2]);
            glTranslatef(0.0f, 0.01f, 0.0f);

            glPushMatrix();
            {
                glScalef(size, size, size);
                glRotatef(-90, 1, 0, 0);
                drawCircle();
                size -= 0.02f;
            }

            glPopMatrix();
            free(colors);
        }

    }
    glPopMatrix();
}

/**
 * Zeichnet das Haus
 */
void drawHouse() {
    drawFreeBlock();
    glColor3f(1.0f, 1.0f, 1.0f);
    setMaterialLightning(1, 1, 1);

    glPushMatrix();
    {
        glPushMatrix();
        {
            // Hauskoerper
            glScalef(0.8f, 1, 0.8);
            drawCube(P_HOUSE);
        }
        glPopMatrix();

        // Dach
        glPushMatrix();
        {
            glTranslatef(0, 0.305, 0.0f);
            glScalef(0.18f, 0.18f, 0.18f);
            glColor3f(houseColors[0], houseColors[1], houseColors[2]);
            setMaterialLightning(houseColors[0], houseColors[1], houseColors[2]);
            drawPyramid();
        }
        glPopMatrix();

    }
    glPopMatrix();
}

/**
 * Zeichnet die Augen des Spielers
 */
void drawPlayerEyes() {

    // Augenbrauen
    setMaterialLightning(0.8f, 0.8f, 0.8f);
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            int angle = i == 0 ? 10 : -10;
            float x = i == 0 ? 0.4f : -0.4f;

            glScalef(0.5, 0.02, 0.2);
            glRotatef(angle, 0, 0, 1);
            glTranslatef(x, 0.5, -1.2);
            drawCube('a');
        }
        glPopMatrix();
    }

    // Augen
    glColor3f(1, 1, 1);
    setMaterialLightning(1, 1, 1);
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float x = i == 0 ? -0.15f : 0.15f;
            glTranslatef(x, 0.0f, -0.2f);
            glScalef(0.1f, 0.01, 0.1f);
            drawSphere();
        }
        glPopMatrix();
    }

    // Pupillen
    glColor3f(0, 0, 0);
    setMaterialLightning(0, 0, 0);
    for (int j = 0; j < 2; ++j) {
        glPushMatrix();
        {
            float x = j == 0 ? -0.15f : 0.15f;
            glTranslatef(x, 0.0f, -0.25f);
            glScalef(0.03f, 0.003, 0.03f);
            drawSphere();
        }
        glPopMatrix();
    }
}

/**
 * Zeichnet den Spielerkopf
 */
void drawPlayerHead() {
    // Kopf
    glPushMatrix();
    {
        glColor3f(0.824f, 0.706f, 0.549f);
        setMaterialLightning(0.824f, 0.706f, 0.549f);
        glScalef(0.5f, 0.07, 0.5f);
        drawSphere();
    }
    glPopMatrix();

    // Augenbrauen, Augen und Pupillen zeichnen
    drawPlayerEyes();

    // Mund
    glPushMatrix();
    {
        glColor3f(1, 1, 1);
        setMaterialLightning(1, 1, 1);

        glTranslatef(0, -0.02f, -0.22f);
        glScalef(1, 0.02f, 0.1f);
        drawCube('a');
    }
    glPopMatrix();

    // Hutdeckel
    glPushMatrix();
    {
        glTranslatef(0, 0.02f, 0);
        glColor3f(0, 0, 0);
        setMaterialLightning(0, 0, 0);
        glScalef(3.5f, 0.02f, 3);
        drawCube('a');
    }
    glPopMatrix();

    // Hutkoerper
    glPushMatrix();
    {
        glTranslatef(0, 0.02f, 0);
        glColor3f(0, 0, 0);
        setMaterialLightning(0, 0, 0);
        glScalef(2.5, 0.15, 2.5);
        drawCube('a');
    }
    glPopMatrix();
}

/**
 * Zeichnet den Spielerkoerper
 */
void drawPlayerBody() {
    glColor3f(0, 0, 0);
    setMaterialLightning(0, 0, 0);

    // Korpus
    glPushMatrix();
    {
        glTranslatef(0, -0.08f, 0);
        glScalef(0.8f, 0.1f, 0.9f);
        drawSphere();
    }
    glPopMatrix();

    // Beine
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xLegPos = i == 0 ? -0.3f : 0.3f;
            glTranslatef(xLegPos, -0.165f, 0);

            glScalef(0.6f, 0.3f, 0.9f);
            drawCube('a');
        }
        glPopMatrix();
    }

    // Schuhe
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xLegPos = i == 0 ? -0.3f : 0.3f;

            glTranslatef(xLegPos, -0.18f, 0);
            glScalef(0.35f, 0.05f, 0.4f);
            drawTetrahedron();
        }
        glPopMatrix();
    }

    // Arme
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xArmPos = i == 0 ? -0.4f : 0.4f;
            int angle = i == 0 ? 125 : -125;

            glTranslatef(xArmPos, -0.09f, 0);
            glRotatef(angle, 0, 0, 1);
            glScalef(0.07f, 0.12, 0.2);
            drawSphere();
        }
        glPopMatrix();
    }

    // "Haende"
    for (int i = 0; i < 2; ++i) {
        glColor3f(0.824f, 0.706f, 0.549f);
        setMaterialLightning(0.824f, 0.706f, 0.549f);

        glPushMatrix();
        {
            float xHandPos = i == 0 ? -0.45f : 0.45f;

            glTranslatef(xHandPos, -0.13f, 0);

            glScalef(0.05f, 0.02, 0.05f);
            drawSphere();
        }
        glPopMatrix();
    }
}

/**
 * Zeichnet den Spieler
 */
void drawPlayer() {
    glPushMatrix();
    {
        glTranslatef(0, 0.143f, 0);
        glScalef(0.08, 0.8f, 0.08f);

        drawPlayerBody();
        drawPlayerHead();
    }
    glPopMatrix();
}
