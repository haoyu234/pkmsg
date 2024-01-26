## pkmsg
1. 无内存申请
2. 简单轻量, 只有 2 个接口
3. 支持变长数组

## example
```c
int main() {
  uint8_t serialized_buf[40960];
  struct stUseItemReq use_item_req = {.itemID = 1001};

  // 编码
  const size_t wpos =
      cToBuf(stUseItemReqObject, &use_item_req, sizeof(use_item_req), serialized_buf, sizeof(serialized_buf));

  // 写入到文件或者网络
  write(socket, serialized_buf, wpos);

  // 解码
  const size_t rpos =
      cFromBuf(stUseItemReqObject, &use_item_req, sizeof(use_item_req), serialized_buf, wpos);

  ...
}
```

## api
```c
// 编码
size_t cToBuf(const struct clColumn *column, const void *addr, size_t size,
              uint8_t *buf, size_t len);

// 解码
size_t cFromBuf(const struct clColumn *column, void *addr, size_t size,
                const uint8_t *buf, size_t len);
   ```
