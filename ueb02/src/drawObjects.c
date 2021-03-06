#include <math.h>
#include "debug.h"
#include "types.h"
#include "helper.h"

// Werte fuer Portal-Animation
float shrinkVal = 0;
GLboolean isIncreasing = GL_TRUE;

GLfloat houseColors[3] = {1.0f, 0.0f, 1.0f};

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
 * Zeichnet ein Rechteck mit der Breite und Hoehe 1.
 */
static void drawSquare() {

    float length = 0.5f;

    glBegin(GL_QUADS);
    {
        // Links unten
        glVertex2f(-length, -length);

        // Rechts unten
        glVertex2f(length, -length);

        // Links oben
        glVertex2f(length, length);

        // Rechts oben
        glVertex2f(-length, length);
    }

    glEnd();
}

/**
 * Zeichnet ein Dreieck
 */
static void drawTriangle() {
    glBegin(GL_TRIANGLES);

    float x = 0.7f;

    // Links unten
    glVertex2f(-x, -x);

    // Rechts unten
    glVertex2f(x, -x);

    // oben
    glVertex2f(0, x);

    glEnd();
}

/**
 * Zeichnet einen Kreis (fuer den Ball)
 */
static void drawCircle() {

    float radius = 1.0f;

    // Um diesen Wert, den Winkel erhoehen, um irgendwann auf 360 zu kommen
    // je kleiner der Wert, desto runder der Ball
    float increaseValue = 0.01f;

    glBegin(GL_POLYGON);

    // Winkel immer minimal erhoehen und somit den Kreis zeichnen
    // 2 * PI entsprechen 360°
    for (float angle = 0; angle < 2 * M_PI; angle += increaseValue) {
        glVertex3f(radius * cosf(angle), radius * sinf(angle), 0);
    }

    glEnd();
}

/**
 * Zeichnet gerade Striche (z.B. fuer die Mauern)
 * @param width  - Breite (wird ignoriert bei isHorizontal == true)
 * @param heigth - Hoehe (wird ignoriert bei isHorizontal == true)
 * @param isHorizontal - True, wenn horizontal gezeichnet werden soll
 */
void drawDash(float width, float heigth, GLboolean isHorizontal) {

    float w = isHorizontal ? 1 : width;
    float h = isHorizontal ? FUGUE_WIDTH : heigth;

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
    float width = BLOCK_SIZE - 0.01f;

    glPushMatrix();
    {
        glScalef(width, BLOCK_SIZE - 0.01f, 1.0f);
        drawSquare();

        // Fugenfarbe
        glColor3f(0.663f, 0.663f, 0.663f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, -FUGUE_HEIGHT * 2, 0.0f);
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
            glTranslatef(0.0f, -FUGUE_HEIGHT * 2.5f, 0);
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
    float width = BLOCK_SIZE - 0.01f;

    float bottom = -0.5f;
    float offset = 0.15f;

    glPushMatrix();
    {
        glScalef(width, BLOCK_SIZE - 0.01f, 1.0f);
        drawSquare();

        // Fugenfarbe
        glColor3f(0.1f, 0.1f, 0.1f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, bottom + offset, 0.0f);
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
            glTranslatef(-0.4f, 0.0f, 0.0f);
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

    glPushMatrix();
    {
        glScalef(BLOCK_SIZE - 0.01f, BLOCK_SIZE - 0.01f, 1.0f);
        drawSquare();
    }

    glPopMatrix();
}

/**
 * Zeichnet die Tuer
 */
void drawDoor() {
    float width = BLOCK_SIZE - 0.3f;
    drawFreeBlock();
    glColor3f(0.600f, 0.240f, 0.100f);

    glPushMatrix();
    {
        glScalef(width, BLOCK_SIZE - 0.01f, 1.0f);
        drawSquare();

        // Braun
        glColor3f(0.1f, 0.1f, 0.1f);

        // Horizontal
        glPushMatrix();
        {
            glTranslatef(0.0f, 0.0, 0.0f);
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
    float width = BLOCK_SIZE - 0.3f;
    drawFreeBlock();

    glPushMatrix();
    {
        glScalef(width, BLOCK_SIZE / 2, 1.0f);
        glRotatef(90, 0.0f, 0.0f, 1.0f);

        glColor3f(0.137f, 0.137f, 0.557f);
        drawTriangle();
    }
    glPopMatrix();
}

/**
 * Zeichnet den blauen Kasten
 * (Ziel fuer die Dreiecke)
 */
void drawFinish() {
    glColor3f(0.137f, 0.137f, 0.557f);

    glPushMatrix();
    {
        glScalef(BLOCK_SIZE - 0.01f, BLOCK_SIZE - 0.01f, 1.0f);
        drawSquare();
    }

    glPopMatrix();
}

/**
 * Zeichnet einen Pfeil fuer den Tuerschalter
 */
void drawDoorSwitchArrow() {
    float width = BLOCK_SIZE - 0.3f;

    double arrowHeadWidth = width / 1.5;
    double arrowHeadHeight = BLOCK_SIZE / 4;
    double arrowBodyWidth = arrowHeadWidth / 2;
    double arrowBodyHeight = arrowHeadHeight / 2;

    glColor3f(0.498f, 1.000f, 0.831f);

    glPushMatrix();
    {
        // Arrow Head
        glPushMatrix();
        {
            glTranslatef(0, 0.05f, 0);
            glScalef(arrowHeadWidth, arrowHeadHeight, 1.0f);
            drawTriangle();
        }
        glPopMatrix();

        // Arrow Body
        glPushMatrix();
        {
            glTranslatef(0.0, -0.009f, 0);
            glScalef(arrowBodyWidth, arrowBodyHeight * 1.5f, 1.0f);
            drawSquare();
        }
        glPopMatrix();
    }
    glPopMatrix();
}

/**
 * Zeichnet den Tuerschalter
 */
void drawDoorSwitch() {
    drawFreeBlock();

    for (int i = 0; i < 2; ++i) {
        int angle = i == 0 ? -45 : -225;
        float pos = i == 0 ? 0.03f : -0.03f;

        glPushMatrix();
        {
            glTranslatef(pos, pos, 0.0f);
            glRotatef(angle, 0, 0, 1);
            drawDoorSwitchArrow();
        }
        glPopMatrix();
    }
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
    if (isIncreasing) {
        shrinkVal += 0.1f / shrinkInterval;
    } else {
        shrinkVal -= 0.1f / shrinkInterval;
    }

    // Wenn das Portal auf 0 ist, wird es vergroessert
    // sonst verkleinert
    if (portalSize <= 0) {
        isIncreasing = GL_TRUE;
    } else if (portalSize >= 1) {
        isIncreasing = GL_FALSE;
    }

    glPushMatrix();
    {
        glScalef(portalSize, portalSize, 0);
        // 5 Farben, 5 Ringe
        for (int i = 0; i < 5; ++i) {
            float *colors = selectColor(i);
            glColor3f(colors[0], colors[1], colors[2]);

            glPushMatrix();
            {
                glScalef(size, size, 0);
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
    glPushMatrix();
    {
        glTranslatef(0, -0.025f, 0.0f);
        glScalef(1.5f, 1.5f, 0);

        // Hauskoerper
        glPushMatrix();
        {
            glTranslatef(0, -0.015f, 0);
            glScalef(0.09f, 0.06f, 1.0f);

            glColor3f(1.0f, 1.0f, 1.0f);
            drawSquare();
            glPopMatrix();
        }

        // Dach
        glPushMatrix();
        {
            glTranslatef(0, 0.05f, 0);
            glScalef(0.08f, 0.05f, 1.0f);
            glColor3f(houseColors[0], houseColors[1], houseColors[2]);
            drawTriangle();
        }
        glPopMatrix();

        // Tuer
        glPushMatrix();
        {
            glTranslatef(0, -0.03f, 0);
            glScalef(0.018f, 0.03f, 1.0f);
            glColor3f(houseColors[0], houseColors[1], houseColors[2]);
            drawSquare();
        }
        glPopMatrix();

        // Fenster
        for (int i = 0; i < 2; ++i) {
            float x = i == 0 ? -0.015f : 0.015f;
            glPushMatrix();
            {
                glTranslatef(x, 0.003, 0);
                glScalef(0.02f, 0.015f, 1.0f);
                glColor3f(0.529f, 0.808f, 0.922f);
                drawSquare();
            }
            glPopMatrix();
        }

        // Tuerknauf
        glPushMatrix();
        {
            glTranslatef(-0.0055f, -0.035f, 0);
            glScalef(0.005f, 0.005, 1.0f);

            glColor3f(0.663f, 0.663f, 0.663f);
            drawSquare();
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
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xEyebrow = i == 0 ? -0.15f : 0.15f;
            int xEyebrowAngle = i == 0 ? 10 : -10;
            glTranslatef(xEyebrow, 0.3f, 0.0f);
            glRotatef(xEyebrowAngle, 0.0f, 0.0f, 1.0f);
            glColor3f(0, 0, 0);
            glScalef(0.16f, 0.05f, 0);
            drawSquare();
        }
        glPopMatrix();
    }

    // Augen
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float x = i == 0 ? -0.15f : 0.15f;
            glTranslatef(x, 0.15f, 0.0f);
            glColor3f(1, 1, 1);
            glScalef(0.07f, 0.07f, 0);
            drawCircle();
        }
        glPopMatrix();
    }

    // Pupillen
    for (int j = 0; j < 2; ++j) {
        glPushMatrix();
        {
            float x = j == 0 ? -0.15f : 0.15f;
            glTranslatef(x, 0.15f, 0.0f);
            glColor3f(0, 0, 0);
            glScalef(0.03f, 0.03f, 0);
            drawCircle();
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
        glScalef(0.5f, 0.5f, 0);
        drawCircle();
    }
    glPopMatrix();

    // Augenbrauen, Augen und Pupillen zeichnen
    drawPlayerEyes();

    // Mund
    glPushMatrix();
    {
        glTranslatef(0, -0.15f, 0.0f);
        glColor3f(1, 1, 1);
        glScalef(0.5f, 0.05f, 0);
        drawSquare();
    }
    glPopMatrix();

    // Hutdeckel
    glPushMatrix();
    {
        glTranslatef(0, 0.65f, 0.0f);
        glColor3f(0, 0, 0);
        glScalef(0.5f, 0.4f, 0);
        drawSquare();
    }
    glPopMatrix();

    // Hutkoerper
    glPushMatrix();
    {
        glTranslatef(0, 0.45f, 0.0f);
        glColor3f(0, 0, 0);
        glScalef(1.0f, 0.05f, 0);
        drawSquare();
    }
    glPopMatrix();
}

/**
 * Zeichnet den Spielerkoerper
 */
void drawPlayerBody() {
    // Korpus
    glPushMatrix();
    {
        glTranslatef(0, -1.1f, 0.0f);
        glColor3f(0, 0, 0);
        glScalef(0.8f, 0.9f, 0);
        drawCircle();
    }
    glPopMatrix();

    // Beine
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xLegPos = i == 0 ? -0.3f : 0.3f;
            glTranslatef(xLegPos, -2.3f, 0.0f);
            glColor3f(0, 0, 0);
            glScalef(0.3f, 0.9f, 0);
            drawSquare();
        }
        glPopMatrix();
    }

    // Schuhe
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        {
            float xLegPos = i == 0 ? -0.5f : 0.5f;

            glTranslatef(xLegPos, -2.7f, 0.0f);
            glColor3f(0, 0, 0);
            glScalef(0.7f, 0.2f, 0);
            drawTriangle();
        }
        glPopMatrix();
    }

    // Arme
    for (int i = 0; i < 2; ++i) {
        glColor3f(0, 0, 0);
        glPushMatrix();
        {
            float xArmPos = i == 0 ? -0.9f : 0.9f;
            int angle = i == 0 ? -60 : 60;

            glTranslatef(xArmPos, -0.9f, 0.0f);
            glRotatef(angle, 0, 0, 1);
            glScalef(0.3f, 0.9f, 0);
            drawSquare();
        }
        glPopMatrix();
    }

    // "Haende"
    for (int i = 0; i < 2; ++i) {
        glColor3f(0.824f, 0.706f, 0.549f);
        glPushMatrix();
        {
            float xHandPos = i == 0 ? -1.4f : 1.4f;
            int angle = i == 0 ? -60 : 60;

            glTranslatef(xHandPos, -1.15f, 0.0f);
            glRotatef(angle, 0, 0, 1);
            glScalef(0.2f, 0.2f, 0);
            drawCircle();
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
        glTranslatef(0, 0.05f, 0.0f);
        glScalef(0.05f, 0.05f, 0);

        drawPlayerBody();

        // Krawatte
        glPushMatrix();
        {
            glTranslatef(0, -0.8f, 0);
            glScalef(0.08f, 0.3, 0);

            glColor3f(1, 1, 1);
            drawTriangle();
        }
        glPopMatrix();

        drawPlayerHead();
    }
    glPopMatrix();
}

