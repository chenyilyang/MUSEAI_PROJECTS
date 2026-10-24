#!/usr/bin/env python3
# 校验 logger_stress 的落盘日志：行数、逐行格式、级别计数、每条应出现的消息恰好一次。
# 用法：verify_log.py <日志文件> <生产者数> <每生产者条数>
# 级别规律（与 logger_stress.cpp 一致）：seq % 5 -> 0:DEBUG 1:INFO 2:WARN 3:ERROR 4:INFO
# Logger 的最低级别为 INFO，DEBUG 全部被过滤，不应出现在文件里。
import re
import sys
from collections import Counter

LEVELS = {0: "DEBUG", 1: "INFO", 2: "WARN", 3: "ERROR", 4: "INFO"}
LINE = re.compile(
    r"^\[\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d{3}\] "
    r"\[(DEBUG|INFO|WARN|ERROR)\] p(\d+) seq (\d+)$"
)


def main():
    path, producers, per = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    expected = Counter()
    for p in range(producers):
        for s in range(per):
            lv = LEVELS[s % 5]
            if lv != "DEBUG":
                expected[(lv, p, s)] += 1
    got = Counter()
    nlines = 0
    with open(path, encoding="utf-8") as f:
        for raw in f:
            nlines += 1
            m = LINE.match(raw.rstrip("\n"))
            if not m:
                print(f"BADLINE {nlines}: {raw!r}")
                return 1
            got[(m.group(1), int(m.group(2)), int(m.group(3)))] += 1
    if got != expected:
        missing = expected - got
        extra = got - expected
        print(f"MISMATCH lines={nlines} missing={sum(missing.values())} extra={sum(extra.values())}")
        for k in list(missing)[:5]:
            print("  missing:", k)
        for k in list(extra)[:5]:
            print("  extra:", k)
        return 1
    per_level = Counter()
    for (lv, _p, _s), c in got.items():
        per_level[lv] += c
    print(f"LOGOK lines={nlines} " +
          " ".join(f"{lv}={per_level[lv]}" for lv in ("INFO", "WARN", "ERROR")))
    return 0


if __name__ == "__main__":
    sys.exit(main())
