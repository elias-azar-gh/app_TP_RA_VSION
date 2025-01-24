
// OpenGL/GLUT
#include <Windows.h>
#include <glfw3.h>

// Main include
#include "main.h"

void error(int error, const char* description)
{
    cout << "GLFW error code: " << error << ", description: " << description << endl;
}


// Resize function
void resize(GLFWwindow* window, GLsizei iWidth, GLsizei iHeight) {
    // Calling ArUco resize
    arucoManager->resize(iWidth, iHeight);

    // not all sizes are allowed. OpenCV images have padding
    // at the end of each line in these that are not aligned to 4 bytes
    if (iWidth * 3 % 4 != 0) {
        iWidth += iWidth * 3 % 4;//resize to avoid padding
    }
    glfwSetWindowSize(window, iWidth, iHeight);
}

// Mouse function
void mouse(GLFWwindow* window, double x, double y) {
    // nothing for now
}

// Keyboard function
void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            //case KEY_ESCAPE:
            exitFunction();
            exit(0);
            break;

        default:
            break;
        }
    }
}

// Loop function
void doWork() {


}

// Idle function
void idle() {

    // Clearing color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Getting current frames
    cap >> curImg;
    if (curImg.empty())
        return;

    // Calling ArUco idle
    arucoManager->idle(curImg);

    // Keyboard manager + waiting for key
    char retKey = cv::waitKey(1);
    if (retKey == KEY_ESCAPE) {
        exitFunction();
        exit(EXIT_SUCCESS);
    }
}


// Initializing OpenGL/GLUt states
void initGL(int argc, char* argv[]) {

    glfwSetErrorCallback(error);
    if (!glfwInit()) exitFunction();
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    //glfwWindowHint(GLFW_DEPTH_BITS, 24);
    window = glfwCreateWindow(widthFrame, heightFrame, "ArUco", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exitFunction();
    }

    // Setting window's initial position
    glfwSetWindowPos(window, 200, 200);

    glfwSetKeyCallback(window, keyboard);
    glfwSetCursorPosCallback(window, mouse);
    glfwSetFramebufferSizeCallback(window, resize);

    glfwMakeContextCurrent(window);

    // Setting up clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // and depth clear value
    glClearDepth(1.0);

    // we define some rendering parameters
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    // and xwe activate backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // We make sure the webcam frame's size to match thatof the OpenGL's window
    arucoManager->resize(widthFrame, heightFrame);
    glfwSetWindowSize(window, widthFrame, heightFrame);


    // Reading a first webcam's frame to make sure of their size
    cap >> curImg;
    // and we scale the camara parameters to match the calibration file with the current resolution (they may be different)
    arucoManager->resizeCameraParams(curImg.size());

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // Getting current frame from the camera
        cap >> curImg;

        // Calling ArUco idle
        arucoManager->idle(curImg);

        // Calling ArUco draw function
        arucoManager->drawScene();

        // check and call events and swap the buffers
        glfwPollEvents();
        glfwSwapBuffers(window);

        // Showing images
        imshow(windowNameCapture, curImg);

        // Keyboard manager + waiting for key
        char retKey = cv::waitKey(1);
        if (retKey == KEY_ESCAPE) {
            exitFunction();
            exit(EXIT_SUCCESS);
        }
    }
}



// Exit function
void exitFunction() {

    // Destroy OpenCV window
    destroyWindow(windowNameCapture);

    // Release capture
    cap.release();

    // Deleting ArUco manager
    if (arucoManager) {
        delete(arucoManager);
        arucoManager = NULL;
    }

    // Deleting GLUT window
    if (window) {
        //glfwDestroyWindow(window);
    }

}

// Main 
int main(int argc, char* argv[])
{
    // print a welcome message, and the OpenCV version
    printf("Welcome to arucoMinimal, using OpenCV version %s (%d.%d.%d)\n",
        CV_VERSION,
        CV_MAJOR_VERSION, CV_MINOR_VERSION, CV_SUBMINOR_VERSION);

    printf("Hot keys: \n"
        "\tESC - quit the program\n");

    // Creating the ArUco object
    arucoManager = new ArUco("camera.yml", 0.105f);
    std::cout << "ArUco OK" << std::endl;

    // Creating the OpenCV capture
    cout << "Entrez l'identifiant de la camera" << endl;
    cin >> cameraID;
    cap.open(cameraID);
    if (!cap.isOpened()) {
        cerr << "Erreur lors de l'initialisation de la capture de la camera !" << endl;
        cerr << "Fermeture..." << endl;
        exit(EXIT_FAILURE);
    }
    else {
        // retrieving a first frame so that the display does not crash
        cap >> curImg;
    }

    // Getting width/height of the image
    widthFrame = cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_WIDTH);
    heightFrame = cap.get(cv::VideoCaptureProperties::CAP_PROP_FRAME_HEIGHT);
    std::cout << "Frame width = " << widthFrame << std::endl;
    std::cout << "Frame height = " << heightFrame << std::endl;

    // OpenCV window 
    windowNameCapture = "Scene";
    cv::namedWindow(windowNameCapture, WINDOW_AUTOSIZE);

    // Exit function
    atexit(exitFunction);

    // OpenGL/GLUT Initialization
    initGL(argc, argv);

    return 0;
}