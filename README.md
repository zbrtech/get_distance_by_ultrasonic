## 单片机控制超声波测距离 ##

---

## 实验进度

> 时间：2016/12/20/0:54
> 最近事情太多了，第一次先实现了对1602显示屏操作的功能<br>
> 时间：2016/12/20/18:52
> 现在先实现了一个大概，主要报警，测距<br>
> 时间：2016/12/21/22:20
> 现在已经做完了，其实做了好几个版本<br>

---

## 功能
- 首先是测定距离，通过发射和回收声波测距离<br>
- 由于没有温度模块，而且时间也不允许，测试距离并不精确<br>
- 设定安全距离<br>
- 小于安全距离发出声音和光警报，距离越小，声音越急促<br>

---

## 主要器件
- 单片51/52
- 超声波模块

---

## 最后来一波吐槽
- 中断方式来读取小键盘的操作难道不是最好，可惜并不知道如何才能在中断中对1602操作而不出现乱七八糟的东西
