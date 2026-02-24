import datetime

def main():
    # 获取当前时间
    now = datetime.datetime.now()
    
    # 定义星期列表 (Python中 weekday() 周一为0，周日为6)
    week_days = ["星期一", "星期二", "星期三", "星期四", "星期五", "星期六", "星期日"]
    
    # 获取星期索引
    day_index = now.weekday()
    
    # 输出结果
    print(f"当前时间: {now.strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"今天是: {week_days[day_index]}")

if __name__ == "__main__":
    main()
