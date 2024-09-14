#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

// Define custom colors
Color DARK_PURPLE = {46, 7, 63, 255};
Color LIGHT_PURPLE = {235, 211, 248, 255};

// Define global variables for game settings
int cell_size = 40;         // Size of each cell in the grid
int cell_count = 20;        // Number of cells in the grid (20x20)
int border_width = 75;      // Width of the border around the game grid
double last_update = 0;     // Tracks time for event updates
bool firstMove = true;      // Flag to track if it's the first move after game reset

// Function to check if a specified time interval has passed
bool eventTriggered(double interval) {
    double current_time = GetTime();
    if (current_time - last_update >= interval) {
        last_update = current_time;
        return true;
    }
    return false;
}

// Class to represent the food that the snake eats
class Food {
public:
    Texture2D texture;       // Texture of the food (an apple image)
    Vector2 coordinate = {2, 4}; // Initial position of the food

    // Generate a random coordinate for the food within the grid
    Vector2 CreateRandomCoordinate() {
        float x = GetRandomValue(0, cell_count - 1);
        float y = GetRandomValue(0, cell_count - 1);
        return Vector2{x, y};
    }

    // Constructor: Loads the food texture and sets a random coordinate
    Food() {
        Image food_img = LoadImage("assets/apple.png");
        texture = LoadTextureFromImage(food_img);
        coordinate = CreateRandomCoordinate();
        UnloadImage(food_img);
    }

    // Destructor: Unloads the texture when the food object is destroyed
    ~Food() {
        UnloadTexture(texture);
    }

    // Draw the food at its current coordinate
    void Draw() {
        DrawTexture(texture, border_width + coordinate.x * cell_size, border_width + coordinate.y * cell_size, WHITE);
    }
};

// Class to represent the snake
class Snake {
public:
    Vector2 direction = {1, 0}; // Initial movement direction of the snake (right)

    // Deque to represent the snake's body as a series of coordinates
    deque<Vector2> snake_body = {
        Vector2{5, 9},
        Vector2{6, 9},
        Vector2{7, 9},
    };

    // Draw each segment of the snake's body
    void Draw() {
        for (int i = 0; i < (int)snake_body.size(); ++i) {
            float x = snake_body[i].x;
            float y = snake_body[i].y;
            Rectangle segment = {border_width + x * cell_size, border_width + y * cell_size, (float)cell_size, (float)cell_size};
            DrawRectangleRounded(segment, 0.5, 6, PURPLE);
        }
    }

    // Update the snake's position by moving the body in the direction of the current head
    void Update() {
        snake_body.pop_back(); // Remove the tail segment
        snake_body.push_front(Vector2Add(snake_body[0], direction)); // Add a new head in the current direction
    }

    // Reset the snake to its initial state
    void Reset() {
        snake_body = {
            Vector2{5, 9},
            Vector2{6, 9},
            Vector2{7, 9},
        };
        direction = {1, 0}; // Ensure the snake moves right initially
    }
};

// Class to manage the overall game state
class Game {
public:
    Snake snake = Snake();   // Snake object
    Food food = Food();      // Food object
    bool pause = false;      // Boolean to track if the game is paused
    int score = 0;           // Player's score
    Sound eat_audio;         // Sound when snake eats food
    Sound hit_audio;         // Sound when snake hits a wall or itself

    // Constructor: Load audio and initialize the audio device
    Game() {
        InitAudioDevice();
        eat_audio = LoadSound("assets/audio/eat.mp3");
        hit_audio = LoadSound("assets/audio/wall.mp3");
    }

    // Destructor: Unload sounds and close the audio device
    ~Game() {
        UnloadSound(eat_audio);
        UnloadSound(hit_audio);
        CloseAudioDevice();
    }

    // End the game and reset the snake and food
    void EndGame() {
        firstMove = true;  // Reset first move tracking
        score = 0;
        snake.Reset();
        food.coordinate = food.CreateRandomCoordinate();
        pause = true;
    }

    // Draw the game elements (snake and food)
    void Draw() {
        food.Draw();
        snake.Draw();
    }

    // Update the game state, including collisions and snake movement
    void Update() {
        if (!pause) {
            snake.Update();
            CheckCollisionWithApple();
            CheckCollisionWithBorder();
            if (!firstMove) // Avoid self-collision check on the first move
                CheckCollisionWithSelf();
        }
    }

    // Check if the snake collides with itself
    void CheckCollisionWithSelf() {
        for (int i = 1; i < (int)snake.snake_body.size(); ++i) {
            if (Vector2Equals(snake.snake_body[0], snake.snake_body[i])) {
                PlaySound(hit_audio);
                EndGame();
            }
        }
    }

    // Check if the snake eats the food
    void CheckCollisionWithApple() {
        if (Vector2Equals(snake.snake_body[0], food.coordinate)) {
            PlaySound(eat_audio);
            snake.snake_body.push_front(Vector2Add(snake.snake_body[0], snake.direction)); // Grow the snake
            food.coordinate = food.CreateRandomCoordinate(); // Move food to a new location
            score += 1;
        }
    }

    // Check if the snake collides with the game border
    void CheckCollisionWithBorder() {
        if (snake.snake_body[0].x == cell_count || snake.snake_body[0].x == -1 || 
            snake.snake_body[0].y == cell_count || snake.snake_body[0].y == -1) {
            PlaySound(hit_audio);
            EndGame();
        }
    }
};

int main() {
    cout << "Starting the game..." << endl;
    
    // Initialize the game window and set the target frame rate
    InitWindow(2 * border_width + cell_size * cell_count, 2 * border_width + cell_size * cell_count, "Snake Game");
    SetTargetFPS(60);

    // Create a Game object
    Game game = Game();

    // Main game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        
        // Update the game state at intervals
        if (eventTriggered(0.1)) {
            game.Update();
        }

        // Handle player input for controlling the snake
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.pause = false;
            firstMove = false; // First valid move is made
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            firstMove = false;
            game.pause = false;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            firstMove = false;
            game.pause = false;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.pause = false;
        }

        // Clear the screen and draw the game elements
        ClearBackground(DARK_PURPLE);
        DrawRectangleLinesEx(Rectangle{(float)border_width - 5, (float)border_width - 5, (float)cell_size * cell_count + 10, (float)cell_size * cell_count + 10}, 5, PINK);
        DrawText("Snake", border_width - 5, 20, 40, PINK);
        DrawText(("Score: " + to_string(game.score)).c_str(), border_width + 300, 20, 30, VIOLET);

        // Draw instruction text at the bottom-right corner
        const char* instructionText = "Press Esc to exit the game";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), instructionText, 20, 1);
        DrawText(instructionText, GetScreenWidth() - (textSize.x + 20), GetScreenHeight() - (textSize.y + 20), 19, PINK);

        game.Draw();
        EndDrawing();
    }

    // Close the game window when done
    CloseWindow();
    return 0;
}
