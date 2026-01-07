#include "MyAccessHub.h"

// ヘッダに書いちゃうと、複数のcppで読み込むヘッダだとリンクエラーになる
MyGameEngine* MyAccessHub::m_engine = nullptr;