#pragma once
 
class Board;
 
// Xóa tất cả hàng đầy, kéo các hàng phía trên xuống.
// Mỗi hàng bị xóa thì khối rơi nhanh hơn (SV5).
// Trả về số hàng đã xóa.
int removeLine(Board& board);
 
// SV5: thời gian chờ giữa 2 lần rơi (ms). Xóa càng nhiều hàng càng nhỏ.
int getFallDelay();
 
