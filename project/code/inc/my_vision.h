#ifndef __MY_VISION_H__
#define __MY_VISION_H__

#include "zf_common_headfile.h"
#include "user_math.h"

#define IMAGE_COL 188
#define IMAGE_ROW 70

#define NEAR    0
#define MEDIUM  1
#define FAR     2

#define LEFT_LOSE_VALUE     0
#define RIGHT_LOSE_VALUE    187

#define LEFT_CIRCULE        0
#define RIGHT_CIRCULE       1

#define LEFT_CROSS          0
#define RIGHT_CROSS         1

//线段类型判断
typedef enum segment_type{
    NULL_segment        = 0,   //未标记状态
    lose_segment        = 1,   //大量缺线
    straight_segment    = 2,   //判断为直线
    arc_segment         = 3,   //判断为圆弧
    corner_segment      = 4    //判断为弯道

}segment_type;


#define IsArcCorner(seg)        (seg.type == arc_segment||seg.type == corner_segment)
#define IsLose(seg)             (seg.type == lose_segment)
#define IsStrai(seg)            (seg.type == straight_segment)
#define IsNull(seg)             (seg.type == NULL_segment)
#define IsArc(seg)              (seg.type == arc_segment)
#define IsCorner(seg)           (seg.type == corner_segment)

//路况判断
typedef enum{  
    NormalRoads,    //正常赛道
    LoseRoads,      //丢失赛道
    CrossRoads,     //十字
    CirculeRoads,   //圆环
    CornerRoads,    //弯道
    ZebraRoads,     //斑马线

}RoadSymbol_type;

//线段判断
typedef struct segment{
    int16 begin;
    int16 end;
    segment_type type;
    uint8 position;
}segment_t;

//路况特征判断结构体
typedef struct{
    segment_t my_segment_L[10];
    point_t feature_p_L[5];
    int FP_n_L;
    int segment_n_L;

    segment_t my_segment_R[10];
    point_t feature_p_R[5];
    int FP_n_R;
    int segment_n_R;
}Feature;

extern Feature F;

#define Vision_GetSegLenghth(seg) (fabs(seg.begin - seg.end + 1))

extern int adaptivePara;
extern uint8 handle_image[IMAGE_ROW][IMAGE_COL];
extern RoadSymbol_type Current_Road;
extern uint8_t Circule_LorR;

//判断序列是否足够长
#define Vision_IsLone(seg) (seg.begin - seg.end>=15)

//预处理自己的摸索（不一定用得上
void Vision_GetMyImage();
void adaptiveThreshold(uint8_t* img_data, uint8_t* output_data, int width, int height, int block, uint8_t clip_value);

//工具
void Vision_ErrorLogin();
void Vision_BroderPrint();
void Vision_set_AdditionalLine(int16 p1,int16 p2,int16 *broder);

//分割边界
void Vision_GetSegment(int16* broder,uint8_t LorR);
//获取元素
void Vision_SymbolJudge();
//解决道路元素
void Vision_RSHandle();
//识别图像特征点
void Vision_BroderFindFP(int16* broder);
//十字处理函数
void Vision_CrossHandle();
//圆环处理函数
void Vision_CirculeHandle();
//弯道处理
void Vision_CornerHandle();
//斑马线处理
void Vision_ZebraHandle();
//打印特征点
void Vision_DrawFP();
//计算一个矩形区域的黑色比例
float Vision_CalBlackRate(uint8 *img_bwp,uint16_t width,uint16_t height,point_t p1,point_t p2);
//计算一个矩形区域的左右对称比例
float Vision_CalSymRate(uint8_t *img,uint16_t width,uint16_t height,point_t p1,point_t p2,uint8_t ud_or_lr);

uint8_t Vision_IsZebra();

#endif