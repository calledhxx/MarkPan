/*
句構：
<head> <body>; head將會是該元素的類型
<body>; 無head，將自動賦予p(段落)元素

<head>：<head名稱>{<屬性key>=<屬性value>,...}
<body>：可以是非「"」或「'」包裝的文字，若未被包裝，則可以使用跳脫功能：
    ...(a)... ：局部形容 
    a => <功能head> <功能body> ：修飾或增加a的外觀或功能
    <= <顯示head> <顯示body> ：插入特定元素至當前元素中
    *a* ：斜體
    **a** ：粗體
    ***a***：粗斜體
    ```<程式碼名稱> <換行>
    a <換行>
    ```：程式碼區塊
    `a`：單行程式碼
    =a=：螢光筆


<功能head>：需要媒介且有功能的Head。媒介如 `=>` 前方的元素。功能head：
color：幫媒介元素染上後方功能body的顏色。無屬性
copy：賦予「點擊就複製後方功能body內容」。無屬性
to：賦予「點擊就跳轉至後方功能body連結」。屬性有target=blink,self分別為：開啟新視窗、當前視窗

<顯示head>：可以直接賦予元素類型的Head。顯示head：
head：提供h1元素。屬性有n=1,2,3,4,5,6分別為：一級字體、二級字體...
p：提供p元素。無屬性
br：提供br元素。無屬性
img：提供img元素。屬性有x=width=<長度單位>;y=heigh=<長度單位>


*/

head Hello, World!

Welcome to use (MarkPan => to https://google.com),a new markup language

Copy the MarkPan version (Click me => copy 0.01)

Time now : <= time
Day of today : <= time{data=day}
Date of today : <= time{data=date}

//It is a comment

/*
It is a comment too
*/

br

img https://www.google.com/favicon.ico => to https://google.com
img{x=30px,y=40px} https://www.google.com/favicon.ico => 
    to{target=blink} https://google.com

I'm *italic*
I'm **bold**
I'm ***both***

```Mione
include "io";

set x = 100;
print(x);
```

It's `Inline Code`

I'm Red => color red;
I'm Red too => color #f00;
I'm Red too too => color #ff0000;
Is there only (I => color #ff0000)'m Red?

<= I can be Red => color red;

=a= => color red //紅色螢光色的背景 + 預設顏色的a字
=(a => color red)= //預設顏色的螢光色背景 + a字染紅
I'm =important= => color red //前方I'm為紅色的字，important為紅色螢光色的背景 + 預設顏色的a字