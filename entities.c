/*******************************************************************************************
*
*   raylib [core] example - Monads
*
*   Example originally created with raylib 5.5, last time updated with raylib 5.5
*
*   Example contributed by James R. (@<H-Art-Src>)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2025 James R, (@<H-Art-Src>)
*
********************************************************************************************/
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define TAG_CHAR_SIZE 128

// Define the structure for a node in the circular linked list
typedef struct Node {
    Model model;
    Vector3 position;
    BoundingBox boundingBox;
    void (*tick)(struct Node* node); // Custom tick function
    void (*onCollision)(struct Node* node , struct Node* other); // Custom collision function
    struct Node* next;
    char tags[TAG_CHAR_SIZE];
} Node;

// Function to create a new node
Node* CreateNode(Model model , Vector3 position , void (*beginPlay)(Node* node)  , void (*tick)(Node* node)  , void (*onCollision)(Node* node , Node* other) )
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->model = model;
    newNode->position = position;
    newNode->boundingBox = GetModelBoundingBox(newNode->model);
    newNode->tick = tick;
    newNode->onCollision = onCollision;
    newNode->next = NULL;
    memset(newNode->tags , 0 , TAG_CHAR_SIZE);
    beginPlay(newNode); // The struct does not have to store the function; it plays exactly once!
    return newNode;
}

// Function to insert a node at the end of the circular linked list
void InsertEnd(Node** head , Model model , Vector3 position , void (*beginPlay)(Node* node)  , void (*tick)(Node* node)  , void (*onCollision)(Node* node , Node* other) )
{
    Node* newNode = CreateNode(model , position , beginPlay , tick , onCollision);
    if (*head == NULL) {
        *head = newNode;
        newNode->next = *head;
    } else {
        Node* temp = *head;
        while (temp->next != *head) {
            temp = temp->next;
        }
        temp->next = newNode;
        newNode->next = *head;
    }
}

// Function to render the models in the circular linked list
void RenderModels(Node* head)
{
    if (head == NULL) return;
    Node* temp = head;
    do {
        DrawModel(temp->model, temp->position, 1.0f, WHITE);
        temp->tick(temp);
        temp = temp->next;
    } while (temp != head);
}

// Function to update the bounding boxes in the circular linked list
void UpdateBoundingBoxes(Node* head)
{
    if (head == NULL) return;
    Node* temp = head;
    do {
        temp->boundingBox = GetModelBoundingBox(temp->model);
        temp->boundingBox.min = Vector3Add(temp->boundingBox.min, temp->position);
        temp->boundingBox.max = Vector3Add(temp->boundingBox.max, temp->position);
        temp = temp->next;
    } while (temp != head);
}

// Function to check for collisions and execute collision functions
void CheckCollisions(Node* head)
{
    if (head == NULL) return;
    Node* temp = head;
    do {
        Node* other = head;
        do {
            if (temp != other && CheckCollisionBoxes(temp->boundingBox, other->boundingBox)) {
                if (temp->onCollision) {
                    temp->onCollision(temp , other);
                }
            }
            other = other->next;
        } while (other != head);
        temp = temp->next;
    } while (temp != head);
}

// Function to free the circular linked list
void FreeList(Node* head) {
    if (head == NULL) return;
    Node* temp = head;
    Node* nextNode;
    do {
        nextNode = temp->next;
        UnloadModel(temp->model);
        free(temp);
        temp = nextNode;
    } while (temp != head);
}

// Example custom functions --------------------------------------------------
//--------------------------------------------------

// Example begin play function
void ExampleBeginPlay(Node* node)
{
    printf("Spawned actor with model at position: (%f, %f, %f), message printed in beginPlay function!\n", node->position.x, node->position.y, node->position.z);
    if (node->position.x < 0)
    {
        node->tags[0] = 'F';
    }
    else
    {
        node->tags[0] = 'B';
    }
}

// Example tick function
void ExampleTick(Node* node)
{
    if (node->position.x < -3.0f)
        node->tags[0] = 'F';
    else if (node->position.x > 3.0f)
        node->tags[0] = 'B';
    node->position.x += (node->tags[0] == 'F')? 0.025f : -0.025f;
}

// Example collision function
void ExampleCollision(Node* node , Node* other)
{
    // Example collision response: Bounce back.
    if (Vector3Distance(node->position , other->position) >= 0.5f)
    {
        DrawModelWires(node->model , node->position , 1.0f , RED);
        DrawSphereWires(Vector3Lerp(node->position , other->position , 0.1f) , 0.5f , 8 , 8 , PURPLE);
    }
    else if (node->tags[0] == 'F' && other->tags[0] == 'B')
    {
        node->tags[0] = 'B';
    }
    else if (node->tags[0] == 'B' && other->tags[0] == 'B')
    {
        node->tags[0] = 'F';
    }
}

//--------------------------------------------------
int main()
{
    // Initialization of the window
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "ECS with Collision Detection.");
    SetTargetFPS(60);

    //Gane variables
    Camera camera = (Camera){ .position = { 0.0f, 10.0f, 10.0f } , .target = { 0.0f, 0.0f, 0.0f } , .up = { 0.0f, 1.0f, 0.0f } , .fovy = 45.0f, CAMERA_PERSPECTIVE };

    // Initialize the circular linked list
    Node* head = NULL;

    // Insert models into the circular linked list
    Model model1 = LoadModel("resources/model1.obj");
    InsertEnd(&head , model1 , (Vector3){ 0.0f, 0.0f, 0.0f } , ExampleBeginPlay , ExampleTick , ExampleCollision);
    InsertEnd(&head , model1 , (Vector3){ 2.0f, 0.0f, 0.0f } , ExampleBeginPlay , ExampleTick , ExampleCollision);
    InsertEnd(&head , model1 , (Vector3){ -2.0f, 0.0f, 0.0f } , ExampleBeginPlay , ExampleTick , ExampleCollision);

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update bounding boxes
        UpdateBoundingBoxes(head);

        // Draw
        BeginDrawing();
            ClearBackground(RAYWHITE);
            BeginMode3D(camera);
                
                // Check collisions
                CheckCollisions(head);
                //Render models
                RenderModels(head);
                
            EndMode3D();

            DrawText("Entity Component System (ECS) with Collision Detection" , 10 , 10 , 20 , DARKGRAY);

        EndDrawing();
    }
    
    // Unload the model.
    UnloadModel(model1);
    
    // Free the circular linked list
    FreeList(head);

    // De-Initialization
    CloseWindow();

    return 0;
}