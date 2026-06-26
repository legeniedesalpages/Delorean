#pragma once

#include "../display_utils.h"

constexpr uint16_t kPaddleMoveIntervalMs = 35;
constexpr uint16_t kBallStepIntervalMs = 45;
constexpr uint16_t kGameOverDurationMs = 1200;
constexpr uint8_t kArkanoidBrickCount = 12;

struct ArkanoidBrick {
  int16_t x;
  int16_t y;
  bool active;
};

struct ArkanoidGameState {
  ArkanoidBrick bricks[kArkanoidBrickCount];
  int16_t paddleX = 50;
  int16_t ballX = 64;
  int16_t ballY = 34;
  int8_t ballVelocityX = 1;
  int8_t ballVelocityY = -1;
  bool gameOver = false;
  unsigned long gameOverStartedMs = 0;
  unsigned long lastPaddleMoveMs = 0;
  unsigned long lastBallStepMs = 0;
};

inline void resetArkanoidBricks(ArkanoidGameState& game) {
  for (uint8_t index = 0; index < kArkanoidBrickCount; ++index) {
    game.bricks[index].x = 6 + (index % 4) * 30;
    game.bricks[index].y = 8 + (index / 4) * 8;
    game.bricks[index].active = true;
  }
}

inline void resetArkanoidGame(ArkanoidGameState& game) {
  game.paddleX = 50;
  game.ballX = 64;
  game.ballY = 34;
  game.ballVelocityX = 1;
  game.ballVelocityY = -1;
  game.gameOver = false;
  game.gameOverStartedMs = 0;
  game.lastPaddleMoveMs = 0;
  game.lastBallStepMs = 0;
  resetArkanoidBricks(game);
}

inline bool allArkanoidBricksCleared(const ArkanoidGameState& game) {
  for (uint8_t index = 0; index < kArkanoidBrickCount; ++index) {
    if (game.bricks[index].active) {
      return false;
    }
  }

  return true;
}

inline void updateArkanoidPaddle(ArkanoidGameState& game, bool moveLeft, bool moveRight, unsigned long nowMs) {
  if ((nowMs - game.lastPaddleMoveMs) < kPaddleMoveIntervalMs) {
    return;
  }

  if (!moveLeft && !moveRight) {
    return;
  }

  game.lastPaddleMoveMs = nowMs;
  if (moveLeft && game.paddleX > 1) {
    game.paddleX -= 3;
  }

  if (moveRight && game.paddleX < (kLogicalWidth - 19)) {
    game.paddleX += 3;
  }
}

inline void updateArkanoidBall(ArkanoidGameState& game, unsigned long nowMs) {
  if ((nowMs - game.lastBallStepMs) < kBallStepIntervalMs) {
    return;
  }

  game.lastBallStepMs = nowMs;

  int16_t nextBallX = game.ballX + game.ballVelocityX;
  int16_t nextBallY = game.ballY + game.ballVelocityY;

  if (nextBallX <= 0 || nextBallX >= (kLogicalWidth - 2)) {
    game.ballVelocityX = -game.ballVelocityX;
    nextBallX = game.ballX + game.ballVelocityX;
  }

  if (nextBallY <= 10) {
    game.ballVelocityY = -game.ballVelocityY;
    nextBallY = game.ballY + game.ballVelocityY;
  }

  const int16_t paddleY = 52;
  if (game.ballVelocityY > 0 && nextBallY >= (paddleY - 1) && nextBallY <= (paddleY + 2) && nextBallX >= (game.paddleX - 1) && nextBallX <= (game.paddleX + 18)) {
    game.ballVelocityY = -1;
    game.ballVelocityX = (nextBallX < game.paddleX + 6) ? -1 : ((nextBallX > game.paddleX + 12) ? 1 : game.ballVelocityX);
    nextBallY = game.ballY + game.ballVelocityY;
  }

  for (uint8_t index = 0; index < kArkanoidBrickCount; ++index) {
    if (!game.bricks[index].active) {
      continue;
    }

    const int16_t brickLeft = game.bricks[index].x;
    const int16_t brickRight = game.bricks[index].x + 23;
    const int16_t brickTop = game.bricks[index].y;
    const int16_t brickBottom = game.bricks[index].y + 4;

    if (nextBallX >= brickLeft && nextBallX <= brickRight && nextBallY >= brickTop && nextBallY <= brickBottom) {
      game.bricks[index].active = false;
      game.ballVelocityY = -game.ballVelocityY;
      nextBallY = game.ballY + game.ballVelocityY;
      break;
    }
  }

  game.ballX = nextBallX;
  game.ballY = nextBallY;

  if (game.ballY > kLogicalHeight) {
    game.gameOver = true;
    game.gameOverStartedMs = nowMs;
  }

  if (allArkanoidBricksCleared(game)) {
    resetArkanoidBricks(game);
  }
}

inline void updateArkanoidGame(ArkanoidGameState& game, bool moveLeft, bool moveRight, unsigned long nowMs) {
  if (game.gameOver) {
    if ((nowMs - game.gameOverStartedMs) >= kGameOverDurationMs) {
      resetArkanoidGame(game);
    }
    return;
  }

  updateArkanoidPaddle(game, moveLeft, moveRight, nowMs);
  updateArkanoidBall(game, nowMs);
}

inline void drawArkanoidPaddle(U8G2& u8g2, int16_t x, int16_t y) {
  u8g2.drawBox(x, toPhysicalY(y), 18, 3);
}

inline void drawArkanoidBrick(U8G2& u8g2, const ArkanoidBrick& brick) {
  u8g2.drawFrame(brick.x, toPhysicalY(brick.y), 24, 5);
  u8g2.drawBox(brick.x + 1, toPhysicalY(brick.y + 1), 22, 3);
}

inline void renderArkanoidGame(U8G2& u8g2, const ArkanoidGameState& game) {
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(2, toPhysicalY(6), "ARKANOID");
  u8g2.drawHLine(0, toPhysicalY(8), 128);

  for (uint8_t index = 0; index < kArkanoidBrickCount; ++index) {
    if (game.bricks[index].active) {
      drawArkanoidBrick(u8g2, game.bricks[index]);
    }
  }

  drawArkanoidPaddle(u8g2, game.paddleX, 52);
  u8g2.drawDisc(game.ballX, toPhysicalY(game.ballY), 1, U8G2_DRAW_ALL);

  if (game.gameOver) {
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawBox(22, toPhysicalY(20), 84, 16);
    u8g2.setDrawColor(0);
    u8g2.drawStr(29, toPhysicalY(31), "PERDU");
    u8g2.setDrawColor(1);
  }
}