//This is my first iteration of a physics solver I've written. I am keeping it single threaded to keep it as simple as possible if someone is reading this and trying to see my logic.

#include <iostream>
#include <raylib.h>
#include <vector>
#include <raymath.h>
#include <cmath>
#include <algorithm>

const Color colors[10] = {
    { 10, 30, 80, 255 }, { 15, 50, 120, 255 }, { 20, 80, 160, 255 },
    { 30, 110, 190, 255 }, { 40, 140, 210, 255 }, { 60, 170, 220, 255 },
    { 80, 200, 230, 255 }, { 100, 220, 240, 255 }, { 140, 240, 250, 255 },
    { 180, 250, 255, 255 }
};
const int WORLD_WIDTH = 1920;
const int WORLD_HEIGHT = 1080;
const int chunkSize = 40; 
const int MAX_BALL_COUNT_PER_CHUNK = 128;
const int GRID_W = (WORLD_WIDTH / chunkSize) + 1;
const int GRID_H = (WORLD_HEIGHT / chunkSize) + 1;

struct Ball {
    float x, y;
    int id;
    float y_vel, x_vel;
    Color color;
    bool held;
    bool ownedByObject;
    float radius;
};


struct Chunk {
    int ids[MAX_BALL_COUNT_PER_CHUNK];
    int count;
};

struct Map {
    std::vector<Ball> balls;
    Chunk grid[GRID_W][GRID_H];
};


void M_RecalculateGrid(Map* map) {
    for (int x = 0; x < GRID_W; x++) {
        for (int y = 0; y < GRID_H; y++) map->grid[x][y].count = 0;
    }

    for (int i = 0; i < (int)map->balls.size(); i++) {
        int cx = (int)(map->balls[i].x / chunkSize);
        int cy = (int)(map->balls[i].y / chunkSize);
        if (cx >= 0 && cx < GRID_W && cy >= 0 && cy < GRID_H) {
            Chunk& c = map->grid[cx][cy];
            if (c.count < MAX_BALL_COUNT_PER_CHUNK) {
                c.ids[c.count++] = i;
            }
            else {
                std::cout<<"WARNING: MAX COLLISION SPHERES PER CHUNK REACHED";
            }
        }
    }
}

void En_CollisionBall(int ballIdx, Map* map) {
    Ball& ball = map->balls[ballIdx];
    int cx = (int)(ball.x / chunkSize);
    int cy = (int)(ball.y / chunkSize);

    for (int nx = cx - 1; nx <= cx + 1; nx++) {
        for (int ny = cy - 1; ny <= cy + 1; ny++) {
            if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;

            Chunk& chunk = map->grid[nx][ny];
            for (int i = 0; i < chunk.count; i++) {
                Ball& other = map->balls[chunk.ids[i]];
                if (other.id <= ball.id) continue;

                float dx = ball.x - other.x;
                float dy = ball.y - other.y;
                float distSq = dx * dx + dy * dy;
                float minFill = ball.radius + other.radius;

                if (distSq < minFill * minFill && distSq > 0.0001f) {
                    float dist = sqrtf(distSq);
                    float overlap = (minFill - dist) * 0.5f;
                    float nx_norm = dx / dist;
                    float ny_norm = dy / dist;

                    ball.x += nx_norm * overlap;
                    ball.y += ny_norm * overlap;
                    other.x -= nx_norm * overlap;
                    other.y -= ny_norm * overlap;

                    float dvx = ball.x_vel - other.x_vel;
                    float dvy = ball.y_vel - other.y_vel;
                    float impact = (dvx * nx_norm + dvy * ny_norm);
                    if (impact < 0) {
                        ball.x_vel -= impact * nx_norm;
                        ball.y_vel -= impact * ny_norm;
                        other.x_vel += impact * nx_norm;
                        other.y_vel += impact * ny_norm;
                    }
                }
            }
        }
    }
     if (ball.y > WORLD_HEIGHT-ball.radius*2) {
        ball.y = WORLD_HEIGHT-ball.radius*2;
        ball.y_vel *= -0.2f;
        if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
    }
     if (ball.y <ball.radius*2) {
        ball.y = ball.radius*2;
        ball.y_vel *= -0.2f;
        if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
    }
    if (ball.x>WORLD_WIDTH-ball.radius*2) {
        ball.x = WORLD_WIDTH-ball.radius*2;
        ball.x_vel *= -0.2f;
        if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
        
    }
    if (ball.x<ball.radius*2) {
        ball.x = ball.radius*2;
        ball.x_vel *= -0.2f;
        if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
        
    }
}


struct App {
    Map* map;
    App() { map = new Map(); }

    void Init() {
        InitWindow(WORLD_WIDTH, WORLD_HEIGHT, "Physics Toy Fix");
        SetTargetFPS(60);
    }
    ~App() {
        delete map;
    }
    void Run() {
    while (!WindowShouldClose()) {
        if (IsMouseButtonDown(0)) {
            map->balls.push_back({ (float)GetMouseX(), (float)GetMouseY(), (int)map->balls.size(), 
                                   0, 0, colors[GetRandomValue(0, 4)], false, false, (float)GetRandomValue(5, 12) });
        }

        const int subSteps = 8; 
        for (int s = 0; s < subSteps; s++) {
            
            for (auto& ball : map->balls) {
                if (!ball.held) {
                    ball.y_vel += 0.5f / subSteps; 
                    ball.x += ball.x_vel / subSteps;
                    ball.y += ball.y_vel / subSteps;
                }
            }

            M_RecalculateGrid(map);

            for (int i = 0; i < (int)map->balls.size(); i++) {
                En_CollisionBall(i, map);
            }
            
        }

        BeginDrawing();
        ClearBackground(BLACK);
        for (auto& b : map->balls) DrawCircle(b.x, b.y, b.radius, b.color);
        DrawFPS(10, 10);
        EndDrawing();
    }
}
};

int main() {
    App app;
    app.Init();
    app.Run();
    return 0;
}