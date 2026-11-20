#include "Line.h"
#include "Board.h"

namespace {

// SV5: tốc độ rơi (ms giữa 2 lần rơi).
constexpr int StartFallDelayMs = 500;
constexpr int FallDelayStepMs = 50;
constexpr int MinFallDelayMs = 100;

int fallDelayMs = StartFallDelayMs;

// SV5: gọi mỗi khi xóa 1 hàng -> rơi nhanh hơn, không nhanh quá mức tối thiểu.
void speedUp() {
    if (fallDelayMs > MinFallDelayMs) {
        fallDelayMs -= FallDelayStepMs;
    }
}

}  // namespace

int getFallDelay() {
    return fallDelayMs;
}

int removeLine(Board& board) {
    // Board đã có sẵn hàm xóa hàng đầy (public), cells_ thì private
    // -> dùng lại hàm đó, không cần sửa Board.h.
    const int clearedLines = board.clearCompletedLines();

    // SV5: mỗi hàng bị xóa -> rơi nhanh hơn.
    for (int i = 0; i < clearedLines; ++i) {
        speedUp();
    }

    return clearedLines;
}