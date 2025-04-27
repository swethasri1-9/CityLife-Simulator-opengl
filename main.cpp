#include <GL/glut.h>
#include <GL/glu.h>
#include <cmath>
#include <cstdlib>
// Camera position & control
float angle = 0.0f, deltaAngle = 0.0f, ratio;
float x = 0.0f, z = 5.0f;
float lx = 0.0f, lz = -1.0f;
float deltaMoveF = 0.0f, deltaMoveS = 0.0f;
float deltaElevate = 0.0f;
float cameraSpeed = 0.1f;
float zoom = 5.0f;
float cloudOffset = 0.0f;
float car1Z = -25.0f; // Car 1 moving along z-axis (vertical road)
float car2X = -25.0f; // Car 2 moving along x-axis (horizontal road)
float rainOffset = 0.0f;  // for moving rain downward
bool isNight = false;

bool isRainy = false;  // Weather toggle: false for sun, true for rain

// Two humans inside the mall
struct Human {
    float x, z;    // position
    float dx, dz;  // small movement direction
};

Human mallHumans[2];


// -------------------- Utility Functions ----------------------
struct Raindrop {
    float x, y, z;
    float speed;
};

const int numDrops = 1000;
Raindrop raindrops[numDrops];
void init() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Initialize raindrops
    for (int i = 0; i < numDrops; ++i) {
        raindrops[i].x = (rand() % 200 - 100) * 1.0f;  // Random X between -100 and 100
        raindrops[i].y = (rand() % 100) + 10.0f;       // Random height between 10 and 110
        raindrops[i].z = (rand() % 200 - 100) * 1.0f;  // Random Z between -100 and 100
        raindrops[i].speed = 0.1f + (rand() % 100) / 1000.0f; // Different falling speed
    }
    // Initialize mall humans
    mallHumans[0] = { 15.0f, -15.0f, 0.01f, 0.0f };  // Moving slowly along +X
    mallHumans[1] = { 17.0f, -17.0f, -0.01f, 0.0f }; // Moving slowly along -X

}


// Function to draw the sun
void reshape(int w, int h) {
    if (h == 0) h = 1;
    ratio = 1.0f * w / h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45, ratio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}
void drawLaneMarkers(float x, float z, float length, bool vertical) {
    glColor3f(1.0f, 1.0f, 1.0f);  // White color for lane markers

    int numMarkers = (int)(length / 4.0f);  // One marker every 4 units
    float markerLength = 2.0f;
    float markerWidth = 0.1f;

    for (int i = 0; i < numMarkers; ++i) {
        glPushMatrix();
        if (vertical) {
            glTranslatef(x, 0.02f, z - length / 2 + i * 4.0f);
            glScalef(markerWidth, 0.01f, markerLength);
        }
        else {
            glTranslatef(x - length / 2 + i * 4.0f, 0.02f, z);
            glScalef(markerLength, 0.01f, markerWidth);
        }
        glutSolidCube(1.0);
        glPopMatrix();
    }
}
void drawHuman() {
    // Body
    glPushMatrix();
    glColor3f(0.8f, 0.5f, 0.3f);  // Shirt color
    glTranslatef(0.0f, 0.9f, 0.0f);
    glScalef(0.3f, 0.6f, 0.2f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Head
    glPushMatrix();
    glColor3f(1.0f, 0.8f, 0.6f);  // Skin color
    glTranslatef(0.0f, 1.4f, 0.0f);
    glutSolidSphere(0.15f, 20, 20);
    glPopMatrix();

    // Left Leg
    glPushMatrix();
    glColor3f(0.2f, 0.2f, 0.8f);  // Pants color
    glTranslatef(-0.07f, 0.4f, 0.0f);
    glScalef(0.1f, 0.5f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Right Leg
    glPushMatrix();
    glColor3f(0.2f, 0.2f, 0.8f);
    glTranslatef(0.07f, 0.4f, 0.0f);
    glScalef(0.1f, 0.5f, 0.1f);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawStreetLamp(float x, float z) {
    // Draw the lamp post (medium height)
    glPushMatrix();
    glColor3f(0.3f, 0.3f, 0.3f); // Gray pole
    glTranslatef(x, 1.5f, z); // Center of pole at 1.5
    glScalef(0.1f, 3.0f, 0.1f); // Pole height = 3.0 units
    glutSolidCube(1.0f);
    glPopMatrix();

    // Draw the lamp bulb (always visible)
    glPushMatrix();
    if (isNight) {
        glColor3f(1.0f, 1.0f, 0.7f); // Night: soft yellow glow
    }

    else {
        glColor3f(0.8f, 0.8f, 0.8f); // Day: gray bulb
    }
    glTranslatef(x, 3.2f, z); // Lamp head just above the pole
    glutSolidSphere(0.25f, 20, 20); // Slightly bigger bulb
    glPopMatrix();
}

void drawMall(float x, float z, float width, float height, float depth) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // Enable transparency

    float halfWidth = width / 2;
    float halfDepth = depth / 2;

    // 1. Draw Floor inside Mall
    glDisable(GL_BLEND);  // Floor opaque
    glColor3f(0.9f, 0.9f, 0.9f);  // Light grey shiny tiles
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.01f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.01f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.01f, z + halfDepth);
    glVertex3f(x - halfWidth, 0.01f, z + halfDepth);
    glEnd();

    // 2. Draw Glass Walls
    glEnable(GL_BLEND);
    glColor4f(0.7f, 0.9f, 1.0f, 0.4f);  // Light blue transparent glass

    // Front Wall
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, height, z - halfDepth);
    glVertex3f(x - halfWidth, height, z - halfDepth);
    glEnd();

    // Back Wall
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height, z + halfDepth);
    glVertex3f(x - halfWidth, height, z + halfDepth);
    glEnd();

    // Left Wall
    glBegin(GL_QUADS);
    glVertex3f(x - halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x - halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x - halfWidth, height, z + halfDepth);
    glVertex3f(x - halfWidth, height, z - halfDepth);
    glEnd();

    // Right Wall
    glBegin(GL_QUADS);
    glVertex3f(x + halfWidth, 0.0f, z - halfDepth);
    glVertex3f(x + halfWidth, 0.0f, z + halfDepth);
    glVertex3f(x + halfWidth, height, z + halfDepth);
    glVertex3f(x + halfWidth, height, z - halfDepth);
    glEnd();

    glDisable(GL_BLEND);

    // 3. Draw Entrance Door
    glColor3f(0.4f, 0.2f, 0.0f);  // Brown color
    float doorWidth = width / 2;
    float doorHeight = height / 3;
    float frontZ = z - halfDepth - 0.01f;
    glBegin(GL_QUADS);
    glVertex3f(x - doorWidth / 2, 0.0f, frontZ);
    glVertex3f(x + doorWidth / 2, 0.0f, frontZ);
    glVertex3f(x + doorWidth / 2, doorHeight, frontZ);
    glVertex3f(x - doorWidth / 2, doorHeight, frontZ);
    glEnd();

    // 4. Draw Mall Board (parallel to left wall, above entrance)
    glPushMatrix();
    glTranslatef(x - width / 2 - 0.05f, 3.0f, z - halfDepth + 2.0f); // Move close to left wall, lift up
    glRotatef(90, 0, 1, 0); // Rotate so it's along the wall
    glScalef(2.0f, 1.0f, 0.1f); // Scale board size
    glColor3f(0.4f, 0.2f, 0.0f); // Dark brown color
    glutSolidCube(1.0f);
    glPopMatrix();

    // 5. Draw "MALL" text (attached to board)
    glPushMatrix();
    glTranslatef(x - width / 2 - 0.04f, 3.1f, z - halfDepth + 2.0f); // Same position but slightly out from wall
    glRotatef(90, 0, 1, 0); // Rotate to match board
    glColor3f(1.0f, 1.0f, 1.0f); // White text

    // Center the text manually
    float textWidth = 4 * 9;  // Roughly 9 pixels per letter
    glRasterPos3f(-textWidth / 200.0f, 0.0f, 0.0f); // Adjust X so text is centered

    const char* mallText = "MALL";
    for (int i = 0; mallText[i] != '\0'; ++i) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, mallText[i]);
    }
    glPopMatrix();

    // 6. Draw Only Mint and Lavender Stores
    float storeWidth = width * 0.25f;
    float storeDepth = depth * 0.25f;
    float storeHeight = 2.5f;  // Taller stores

    
   // Store 1 - Right-Back Corner (Pastel Mint)
    glPushMatrix();
    glTranslatef(x + halfWidth - storeWidth / 2 - 0.5f, storeHeight / 4, z + halfDepth - storeDepth / 2 - 0.5f);  // fixed Y-axis!
    glScalef(storeWidth, storeHeight, storeDepth);
    glColor3f(0.7f, 1.0f, 0.8f); // 🌿 Pastel Mint Green
    glutSolidCube(0.6f);
    glPopMatrix();

    // Store 2 - Right-Front Corner (Pastel Lavender)
    glPushMatrix();
    glTranslatef(x + halfWidth - storeWidth / 2 - 0.5f, storeHeight / 4, z - halfDepth + storeDepth / 2 + 0.5f);  // fixed Y-axis!
    glScalef(storeWidth, storeHeight, storeDepth);
    glColor3f(0.85f, 0.8f, 1.0f); // 🌸 Pastel Lavender
    glutSolidCube(0.6f);
    glPopMatrix();

    // 7. Draw Humans inside the Mall
    for (int i = 0; i < 2; ++i) {
        glPushMatrix();
        glTranslatef(mallHumans[i].x, 0.0f, mallHumans[i].z);
        drawHuman();
        glPopMatrix();
    }


}




void drawBuilding(float x, float z, float width, float height, float depth) {
    glPushMatrix();
    glColor3f(0.6, 0.6, 0.8); // Building color
    glTranslatef(x, height / 2, z);
    glScalef(width, height, depth);
    glutSolidCube(1);
    glPopMatrix();

    float offset = 0.01f;
    float frontZ = z - depth / 2 - offset;
    float backZ = z + depth / 2 + offset;
    float sideXLeft = x - width / 2 - offset;
    float sideXRight = x + width / 2 + offset;

    // ----- Door on Front Face -----
    glColor3f(0.4f, 0.2f, 0.0f); // Door color
    float doorWidth = width / 2.5;
    float doorHeight = height / 5;
    glBegin(GL_QUADS);
    glVertex3f(x - doorWidth / 2, 0.0f, frontZ);
    glVertex3f(x + doorWidth / 2, 0.0f, frontZ);
    glVertex3f(x + doorWidth / 2, doorHeight, frontZ);
    glVertex3f(x - doorWidth / 2, doorHeight, frontZ);
    glEnd();

    // ----- Window Color -----
    glColor3f(0.8f, 0.9f, 1.0f); // Light blue windows
    float winWidth = width / 4.5;
    float winHeight = height / 8;

    // ----- Windows on Front Face -----
    for (int row = 0; row < 3; row++) {
        for (int col = -1; col <= 1; col += 2) {
            float winX = x + col * width / 3;
            float winY = doorHeight + (row + 1) * (height / 6);
            glBegin(GL_QUADS);
            glVertex3f(winX - winWidth / 2, winY - winHeight / 2, frontZ);
            glVertex3f(winX + winWidth / 2, winY - winHeight / 2, frontZ);
            glVertex3f(winX + winWidth / 2, winY + winHeight / 2, frontZ);
            glVertex3f(winX - winWidth / 2, winY + winHeight / 2, frontZ);
            glEnd();
        }
    }

    // ----- Windows on Left Side -----
    for (int row = 0; row < 3; row++) {
        for (int col = -1; col <= 1; col += 2) {
            float winZ = z + col * depth / 3;
            float winY = doorHeight + (row + 1) * (height / 6);
            glBegin(GL_QUADS);
            glVertex3f(sideXLeft, winY - winHeight / 2, winZ - winWidth / 2);
            glVertex3f(sideXLeft, winY - winHeight / 2, winZ + winWidth / 2);
            glVertex3f(sideXLeft, winY + winHeight / 2, winZ + winWidth / 2);
            glVertex3f(sideXLeft, winY + winHeight / 2, winZ - winWidth / 2);
            glEnd();
        }
    }

    // ----- Windows on Right Side -----
    for (int row = 0; row < 3; row++) {
        for (int col = -1; col <= 1; col += 2) {
            float winZ = z + col * depth / 3;
            float winY = doorHeight + (row + 1) * (height / 6);
            glBegin(GL_QUADS);
            glVertex3f(sideXRight, winY - winHeight / 2, winZ - winWidth / 2);
            glVertex3f(sideXRight, winY - winHeight / 2, winZ + winWidth / 2);
            glVertex3f(sideXRight, winY + winHeight / 2, winZ + winWidth / 2);
            glVertex3f(sideXRight, winY + winHeight / 2, winZ - winWidth / 2);
            glEnd();
        }
    }

    // -----  Windows on Back Face (NEW!!) -----
    for (int row = 0; row < 3; row++) {
        for (int col = -1; col <= 1; col += 2) {
            float winX = x + col * width / 3;
            float winY = doorHeight + (row + 1) * (height / 6);
            glBegin(GL_QUADS);
            glVertex3f(winX - winWidth / 2, winY - winHeight / 2, backZ);
            glVertex3f(winX + winWidth / 2, winY - winHeight / 2, backZ);
            glVertex3f(winX + winWidth / 2, winY + winHeight / 2, backZ);
            glVertex3f(winX - winWidth / 2, winY + winHeight / 2, backZ);
            glEnd();
        }
    }
}

void drawPark(float x, float z, float size) {
    glPushMatrix();
    glColor3f(0.0, 0.8, 0.0);
    glTranslatef(x, 0.01, z);
    glScalef(size, 0.01, size);
    glutSolidCube(1);
    glPopMatrix();
}

void drawRoad(float x, float z, float width, float depth) {
    glPushMatrix();
    glColor3f(0.3, 0.3, 0.3);
    glTranslatef(x, 0, z);
    glScalef(width, 0.01, depth);
    glutSolidCube(1);
    glPopMatrix();
}
void drawCloud(float baseX, float baseY, float baseZ) {
    glPushMatrix();
    glTranslatef(baseX, baseY, baseZ);
    glColor3f(1.0f, 1.0f, 1.0f); // White color for clouds


    for (int i = 0; i < 5; ++i) {
        glPushMatrix();
        glTranslatef(i * 2.0f, (i % 2) * 0.5f, 0.0f);
        glScalef(2.0f, 1.0f, 1.0f);
        glutSolidSphere(1.0f, 20, 20);
        glPopMatrix();
    }

    glPopMatrix();
}


void drawSun() {
    glPushMatrix();
    if (isNight) {
        // Night: Draw a white moon
        glColor3f(0.9f, 0.9f, 1.0f);  // Light blueish white
        glTranslatef(20.0f, 25.0f, 50.0f);
        glutSolidSphere(2.5f, 30, 30);
    }
    else {
        // Day: Draw yellow glowing sun
        glColor4f(1.0f, 1.0f, 0.0f, 0.4f);  // Yellow transparent glow
        glTranslatef(20.0f, 25.0f, 50.0f);
        glutSolidSphere(3.0f, 30, 30);
        glPopMatrix();

        glPushMatrix();
        glColor3f(1.0f, 1.0f, 0.0f);  // Solid yellow Sun
        glTranslatef(0.0f, 100.0f, -400.0f);
        glutSolidSphere(7.0f, 30, 30);
    }
    glPopMatrix();
}

// Function to draw rain
void drawRain() {
    glColor4f(0.25f, 0.41f, 0.88f, 0.9f);  // Soft light blue raindrops

    for (int i = 0; i < numDrops; ++i) {
        glPushMatrix();
        glTranslatef(raindrops[i].x, raindrops[i].y, raindrops[i].z);
        glScalef(0.1f, 0.3f, 0.1f);  // Stretch into vertical droplet shape
        glutSolidSphere(1.0, 8, 8);   // Sphere stretched becomes a droplet
        glPopMatrix();
    }
}




void drawCar() {
    glPushMatrix();
    glScalef(1.5f, 0.5f, 0.8f);
    glColor3f(1.0f, 0.0f, 0.0f); // Red body
    glutSolidCube(1.0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f);
    glScalef(1.2f, 0.4f, 0.6f);
    glColor3f(0.8f, 0.0f, 0.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    GLUquadric* quad = gluNewQuadric();
    glColor3f(0.1f, 0.1f, 0.1f);  // Black wheels

    float wheelPos[4][3] = {
        { 0.6f, -0.25f,  0.35f},
        {-0.6f, -0.25f,  0.35f},
        { 0.6f, -0.25f, -0.35f},
        {-0.6f, -0.25f, -0.35f}
    };

    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glTranslatef(wheelPos[i][0], wheelPos[i][1], wheelPos[i][2]);
        glRotatef(90, 0, 1, 0);
        gluCylinder(quad, 0.1f, 0.1f, 0.2f, 12, 1);
        glPopMatrix();
    }

    gluDeleteQuadric(quad);
}

//void drawSkyWithClouds() {
//    glPushMatrix();
//    if (isNight)
//        glColor3f(0.05f, 0.05f, 0.2f);  // Dark blue sky at night
//    else {
//        glColor3f(0.5f, 0.8f, 1.0f);
//        for (int i = -2; i <= 2; i++) {
//            for (int j = -2; j <= 2; j++) {
//                glPushMatrix();
//                glTranslatef(i * 30.0f + cloudOffset * 0.5f, 12.0f + (i + j) % 3, j * 30.0f);
//                drawCloud(0.0f, 0.0f, 0.0f);
//                glPopMatrix();
//            }
//        }
//    }
//    // Light blue sky during day
//    //glColor3f(0.5f, 0.8f, 1.0f);
//    glTranslatef(x, zoom - 5, z);
//    GLUquadric* quad = gluNewQuadric();
//    gluQuadricNormals(quad, GLU_SMOOTH);
//    gluSphere(quad, 200.0, 32, 32);
//    gluDeleteQuadric(quad);
//    glPopMatrix();
//    if (!isRainy) {
//        drawSun();  // Only draw the sun if it's not rainy
//    }
//    else {
//        drawRain();  // If it's rainy, draw rain instead
//    }
//
//    /*for (int i = -2; i <= 2; i++) {
//        for (int j = -2; j <= 2; j++) {
//            glPushMatrix();
//            glTranslatef(i * 30.0f + cloudOffset * 0.5f, 12.0f + (i + j) % 3, j * 30.0f);
//            drawCloud(0.0f, 0.0f, 0.0f);
//            glPopMatrix();
//        }
//    }*/
//}
void drawSkyWithClouds() {
    glPushMatrix();

    if (isNight) {
        glColor3f(0.05f, 0.05f, 0.2f);  // Dark blue sky at night
    }
    else {
        glColor3f(0.5f, 0.8f, 1.0f);    // Light blue sky during day
    }

    glTranslatef(x, zoom - 5, z);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, 200.0, 32, 32);    // Draw the sky sphere
    gluDeleteQuadric(quad);
    glPopMatrix();

    if (!isNight) {
        // Only draw clouds if it's daytime
        for (int i = -2; i <= 2; i++) {
            for (int j = -2; j <= 2; j++) {
                glPushMatrix();
                glTranslatef(i * 30.0f + cloudOffset * 0.5f, 12.0f + (i + j) % 3, j * 30.0f);
                drawCloud(0.0f, 0.0f, 0.0f);
                glPopMatrix();
            }
        }
    }

    if (!isRainy) {
        drawSun();  // Only draw the sun if it's not rainy
    }
    else {
        drawRain();  // If it's rainy, draw rain instead
    }
}

// -------------------- Main Draw Function ----------------------
void drawCity() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(x, zoom, z, x + lx, zoom, z + lz, 0.0f, 1.0f, 0.0f);
    drawSkyWithClouds();
    if (isNight)
        glColor3f(0.1f, 0.1f, 0.1f);  // Dark ground at night
    else
        glColor3f(0.5, 0.8, 0.9);      // Light ground during day


    glColor3f(0.5, 0.8, 0.9);
    glBegin(GL_QUADS);
    glVertex3f(-100.0f, 0.0f, -100.0f);
    glVertex3f(-100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, 100.0f);
    glVertex3f(100.0f, 0.0f, -100.0f);
    glEnd();

    drawRoad(0, 0, 2, 50);
    drawLaneMarkers(0, 0, 50, true); // Vertical lane on main road

    drawRoad(-10, 0, 2, 50);
    drawLaneMarkers(-10, 0, 50, true); // Vertical lane

    drawRoad(10, 0, 2, 50);
    drawLaneMarkers(10, 0, 50, true); // Vertical lane

    drawRoad(0, -10, 50, 2);
    drawLaneMarkers(0, -10, 50, false); // Horizontal lane

    drawRoad(0, 10, 50, 2);
    drawLaneMarkers(0, 10, 50, false); // Horizontal lane


    glPushMatrix();
    glTranslatef(0.0f, 0.5f, car1Z);
    drawCar();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(car2X, 0.5f, -10.0f);
    glRotatef(90, 0, 1, 0);
    drawCar();
    glPopMatrix();
    for (float i = -40; i <= 40; i += 10) {
        drawStreetLamp(-2.0f, i); // Left side lamps
        drawStreetLamp(2.0f, i);  // Right side lamps
    }

    drawBuilding(-5, -5, 3, 6, 3);
    drawBuilding(5, -5, 3, 5, 3);
    drawBuilding(-5, 5, 3, 4, 3);
    drawBuilding(5, 5, 3, 7, 3);

    drawPark(-15, -15, 6);
    drawPark(15, 15, 6);

    drawMall(15, -15, 8, 5, 10);  // Position and size of mall

    glutSwapBuffers();
}

// -------------------- Input Handlers ----------------------
void pressKey(int key, int x1, int y) {
    switch (key) {
    case GLUT_KEY_LEFT: deltaAngle = -0.01f; break;
    case GLUT_KEY_RIGHT: deltaAngle = 0.01f; break;
    case GLUT_KEY_UP: deltaElevate = 0.2f; break;
    case GLUT_KEY_DOWN: deltaElevate = -0.2f; break;
    }
}

void releaseKey(int key, int x1, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
    case GLUT_KEY_RIGHT:
        deltaAngle = 0.0f;
        break;
    case GLUT_KEY_UP:
    case GLUT_KEY_DOWN:
        deltaElevate = 0.0f;
        break;
    }
}

void keyboard(unsigned char key, int x1, int y1) {
    switch (key) {
    case 'w': deltaMoveF = 1.0f; break;
    case 's': deltaMoveF = -1.0f; break;
    case 'a': deltaMoveS = 1.0f; break;
    case 'd': deltaMoveS = -1.0f; break;
        //case 'q': zoom -= 0.5f; break;
        //case 'e': zoom += 0.5f; break;
    case 'r':
        isRainy = !isRainy;   // Toggle Night/Day mode
        break;
    case 'q':
        isNight = !isNight;   // Toggle Night/Day mode
        break;
    }
}

void keyboardUp(unsigned char key, int x1, int y1) {
    switch (key) {
    case 'w':
    case 's': deltaMoveF = 0.0f; break;
    case 'a':
    case 'd': deltaMoveS = 0.0f; break;
    }
}

void update(int value) {
    angle += deltaAngle;
    lx = sin(angle);
    lz = -cos(angle);

    x += deltaMoveF * lx * cameraSpeed;
    z += deltaMoveF * lz * cameraSpeed;

    x += deltaMoveS * lz * cameraSpeed;
    z -= deltaMoveS * lx * cameraSpeed;

    zoom += deltaElevate;
    if (zoom < 2.0f) zoom = 2.0f;
    if (zoom > 30.0f) zoom = 30.0f;

    cloudOffset += 0.02f;
    if (cloudOffset > 100.0f) cloudOffset = -100.0f;

    car1Z += 0.1f;
    if (car1Z > 25.0f) car1Z = -25.0f;

    car2X += 0.1f;
    if (car2X > 25.0f) car2X = -25.0f;

    // Animate humans inside the mall
    for (int i = 0; i < 2; ++i) {
        mallHumans[i].x += mallHumans[i].dx;
        mallHumans[i].z += mallHumans[i].dz;

        // Boundary conditions inside mall (simple bounce)
        if (mallHumans[i].x < 13.0f || mallHumans[i].x > 17.0f) {
            mallHumans[i].dx = -mallHumans[i].dx;  // Reverse direction
        }
        if (mallHumans[i].z < -18.0f || mallHumans[i].z > -12.0f) {
            mallHumans[i].dz = -mallHumans[i].dz;  // Reverse direction
        }
    }

    if (isRainy) {
        for (int i = 0; i < numDrops; ++i) {
            raindrops[i].y -= raindrops[i].speed * 5.0f;  // Fall based on speed
            if (raindrops[i].y < 0) {  // Hit ground
                raindrops[i].y = (rand() % 100) + 50.0f; // Reset high up
                raindrops[i].x = (rand() % 200 - 100) * 1.0f;
                raindrops[i].z = (rand() % 200 - 100) * 1.0f;
            }
        }
    }


    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}



// -------------------- Main Program ----------------------

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D City with Weather Toggle");

    init();
    glutDisplayFunc(drawCity);
    glutReshapeFunc(reshape);
    glutSpecialFunc(pressKey);
    glutSpecialUpFunc(releaseKey);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}



