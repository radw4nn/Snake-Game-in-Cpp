#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color DARK_PURPLE = {46, 7, 63, 255};
Color LIGHT_PURPLE = {235, 211, 248, 255};

int cell_size = 40;
int cell_count = 20;
int border_width = 75;
double last_update = 0;
bool firstMove = true; // Track if it's the first move after reset

bool eventTriggered(double interval)
{
    double current_time = GetTime();
    if (current_time - last_update >= interval)
    {
        last_update = current_time;
        return true;
    }

    return false;
}

class Food
{
public:
    Texture2D texture;
    Vector2 coordinate = {2, 4};

    Vector2 CreateRandomCoordinate()
    {
        float x = GetRandomValue(0, cell_count - 1);
        float y = GetRandomValue(0, cell_count - 1);
        return Vector2{x, y};
    }

    Food()
    {
        Image food_img = LoadImage("assets/apple.png");
        texture = LoadTextureFromImage(food_img);
        coordinate = CreateRandomCoordinate();
        UnloadImage(food_img);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, border_width + coordinate.x * cell_size, border_width + coordinate.y * cell_size, WHITE);
    }
};

class Snake
{
public:
    Vector2 direction = {1, 0};

    deque<Vector2> snake_body = {
        Vector2{5, 9},
        Vector2{6, 9},
        Vector2{7, 9},
    };

    void Draw()
    {
        for (int i = 0; i < (int)snake_body.size(); ++i)
        {
            float x = snake_body[i].x;
            float y = snake_body[i].y;
            Rectangle segment = {border_width + x * cell_size, border_width + y * cell_size, (float)cell_size, (float)cell_size};
            DrawRectangleRounded(segment, 0.5, 6, PURPLE);
        }
    }

    void Update()
    {
        snake_body.pop_back();
        snake_body.push_front(Vector2Add(snake_body[0], direction));
    }

    void Reset()
    {
        snake_body = {
            Vector2{5, 9},
            Vector2{6, 9},
            Vector2{7, 9},
        };
        direction = {1, 0}; // Ensure the snake moves right initially
    }
};

class Game
{
public:
    Snake snake = Snake();
    Food food = Food();
    bool pause = false;
    int score = 0;
    Sound eat_audio;
    Sound hit_audio;

    Game()
    {
        InitAudioDevice();
        eat_audio = LoadSound("assets/audio/eat.mp3");
        hit_audio = LoadSound("assets/audio/wall.mp3");
    }

    ~Game()
    {
        UnloadSound(eat_audio);
        UnloadSound(hit_audio);
        CloseAudioDevice();
    }

    void EndGame()
    {
        firstMove = true; // Reset first move tracking
        score = 0;
        snake.Reset();
        food.coordinate = food.CreateRandomCoordinate();
        pause = true;
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
    }

    void Update()
    {
        if (!pause)
        {
            snake.Update();
            CheckCollisionWithApple();
            CheckCollisionWithBorder();
            if (!firstMove) // Avoid self-collision check on the first move
                CheckCollisionWithSelf();
        }
    }

    void CheckCollisionWithSelf()
    {
        for (int i = 1; i < (int)snake.snake_body.size(); ++i)
        {
            if ((Vector2Equals(snake.snake_body[0], snake.snake_body[i])))
            {
                PlaySound(hit_audio);
                EndGame();
            }
        }
    }

    void CheckCollisionWithApple()
    {
        if ((Vector2Equals(snake.snake_body[0], food.coordinate)))
        {
            PlaySound(eat_audio);
            snake.snake_body.push_front(Vector2Add(snake.snake_body[0], snake.direction));
            food.coordinate = food.CreateRandomCoordinate();
            score += 1;
        }
    }

    void CheckCollisionWithBorder()
    {
        if ((snake.snake_body[0].x == cell_count) || (snake.snake_body[0].x == -1))
        {
            PlaySound(hit_audio);
            EndGame();
        }

        if ((snake.snake_body[0].y == cell_count) || (snake.snake_body[0].y == -1))
        {
            PlaySound(hit_audio);
            EndGame();
        }
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * border_width + cell_size * cell_count, 2 * border_width + cell_size * cell_count, "Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        if (eventTriggered(0.1))
        {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP))
        {
            if (!(game.snake.direction.y == 1))
            {
                game.snake.direction = {0, -1};
                game.pause = false;
                firstMove = false; // First valid move is made
            }
        }

        if (IsKeyPressed(KEY_DOWN))
        {
            if (!(game.snake.direction.y == -1))
            {
                game.snake.direction = {0, 1};
                firstMove = false;
                game.pause = false;
            }
        }
        if (IsKeyPressed(KEY_LEFT))
        {
            if (!(game.snake.direction.x == 1))
            {
                game.snake.direction = {-1, 0};
                firstMove = false;
                game.pause = false;
            }
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
            if (!(game.snake.direction.x == -1))
            {
                game.snake.direction = {1, 0};
                game.pause = false;
            }
        }

        ClearBackground(DARK_PURPLE);
        DrawRectangleLinesEx(Rectangle{(float)border_width - 5, (float)border_width - 5, (float)cell_size * (float)cell_count + 10, (float)cell_size * (float)cell_count + 10}, 5, PINK);
        DrawText("Snake", border_width - 5, 20, 40, PINK);
        DrawText(("Score: " + to_string(game.score)).c_str(), border_width + 300, 20, 30, VIOLET);

        // Draw the instruction text at the bottom-right corner
        const char *instructionText = "Press Esc to exit the game";
        Vector2 textSize = MeasureTextEx(GetFontDefault(), instructionText, 20, 1);                                       // Measure text size
        DrawText(instructionText, GetScreenWidth() - (textSize.x + 20), GetScreenHeight() - (textSize.y + 20), 19, PINK); // Draw text
        game.Draw();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
