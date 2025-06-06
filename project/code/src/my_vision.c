#include "my_vision.h"
#include "zf_device_tft180.h"
#include "string.h"

int block = 7;
int adaptivePara = 6;

RoadSymbol_type Current_Road;

Feature F;

/**
 * @brief 根据赛道元素选择处理
 * 
 */
void Vision_RSHandle()
{
    switch (Current_Road)
    {
    case LoseRoads:
        Current_Road = NormalRoads;
        
        break;

    case NormalRoads: 
        ips200_show_string(200,50,"Nor");
        break;

    case CrossRoads:
        Vision_CrossHandle();
        ips200_show_string(200,50,"Cro");
        break;
    
    case CirculeRoads:
        Vision_CirculeHandle();
        ips200_show_string(200,50,"Cir");
        break;

    case CornerRoads:
        Vision_CornerHandle();
        ips200_show_string(200,50,"Cor");
        break;

    case ZebraRoads:
        Vision_ZebraHandle();
        ips200_show_string(200,50,"Zeb");
        break;

    default:
        break;
    }

}

/**
 * @brief 检测路段状况
 * 
 */
//双边弧
#define CornerState1    (IsArcCorner(F.my_segment_L[0])&&IsArcCorner(F.my_segment_L[0])&&F.segment_n_L == 1&&F.segment_n_R == 1)
//双边弧+缺陷
#define CornerState2    (IsArcCorner(F.my_segment_L[0])&&IsArcCorner(F.my_segment_L[0])\
                        &&IsLose(F.my_segment_L[1])&&IsLose(F.my_segment_R[1])\
                        && F.segment_n_L == 2 && F.my_segment_R == 2) 
//单边缺线
#define CornerState3    (IsArcCorner(F.my_segment_L[0])&&IsLose(F.my_segment_L[1])&&IsLose(F.my_segment_R[0])&&F.segment_n_R == 1\
                       ||IsArcCorner(F.my_segment_R[0])&&IsLose(F.my_segment_R[1])&&IsLose(F.my_segment_L[0])&&F.my_segment_L == 1)

#define CrossCon1       (F.FP_n_L == 2&&F.FP_n_R == 2)
#define CrossCon2       ((F.FP_n_L == 1&&F.FP_n_R == 2)||(F.FP_n_R == 1&&F.FP_n_L == 2))
#define CrossCon3       ((IsLose(F.my_segment_L[0])&&F.segment_n_L == 1&&F.FP_n_L == 0&&F.FP_n_R == 2)||\
                         (IsLose(F.my_segment_R[0])&&F.segment_n_R == 1&&F.FP_n_R == 0&&F.FP_n_L == 2))
//十字特殊情况：识别到了圆环的弯道
#define CrossCon4       (((IsLose(F.my_segment_R[0])&&!IsNull(F.my_segment_R[1]))&&(F.segment_n_L <= 2&&IsArc(F.my_segment_L[0])&&F.FP_n_L))||\
                         ((IsLose(F.my_segment_L[0])&&!IsNull(F.my_segment_L[1]))&&(F.segment_n_R <= 2&&IsArc(F.my_segment_R[0])&&F.FP_n_L)))

void Vision_SymbolJudge()
{
    //获取边界分段信息
    Vision_GetSegment(Image_S.leftBroder,1);
    Vision_GetSegment(Image_S.rightBroder,0);
    
    //寻找特征点
    Vision_BroderFindFP(Image_S.leftBroder);
    Vision_BroderFindFP(Image_S.rightBroder);

    char str[100];
    rt_sprintf(str,"left_p:%d",F.FP_n_L);
    ips200_show_string(200,35,str);
    rt_sprintf(str,"right_p:%d",F.FP_n_R);
    ips200_show_string(200,20,str);

    //只有当道路情况为正常道路时才需要进行判断
    if(Current_Road == NormalRoads){
        if(Vision_IsZebra()){
            Current_Road = ZebraRoads;
        }
        else if( (F.segment_n_L == 1 && IsStrai(F.my_segment_L[0]) && IsLose(F.my_segment_R[1]))||
            (F.segment_n_R == 1 && IsStrai(F.my_segment_R[0]) && IsLose(F.my_segment_L[1])) )
            Current_Road = CirculeRoads;

        else if(CornerState1||CornerState2||CornerState3) 
            Current_Road = CornerRoads;

        else if(CrossCon1||CrossCon2||CrossCon3||CrossCon4)
            Current_Road = CrossRoads;
        
        else if(F.segment_n_L == 1&&F.segment_n_R == 1 &&
                IsLose(F.my_segment_L[0])&&IsLose(F.my_segment_R[0]))
            Current_Road = LoseRoads;

        else 
            Current_Road = NormalRoads;
        
        
    }
        
}

/**
 * @brief 打印特征点
*/
void Vision_DrawFP(){
    
    for(int i = 0;i<F.FP_n_L;i++){
//        ips200_draw_point(F.feature_p_L[i].y*158/188,78-(imgRow-1)+F.feature_p_L[i].x,RGB565_RED);
//        if(78-(imgRow-1)+F.feature_p_L[i].x+1 < 128)
//            ips200_draw_point(F.feature_p_L[i].y*158/188,78-(imgRow-1)+F.feature_p_L[i].x+1,RGB565_RED);
//        if(78-(imgRow-1)+F.feature_p_L[i].x-1 >= 0)
//            ips200_draw_point(F.feature_p_L[i].y*158/188,78-(imgRow-1)+F.feature_p_L[i].x-1,RGB565_RED);
//        if((F.feature_p_L[i].y+1)*158/188 < 160)
//            ips200_draw_point((F.feature_p_L[i].y+1)*158/188,78-(imgRow-1)+F.feature_p_L[i].x,RGB565_RED);
//        if((F.feature_p_L[i].y+1)*158/188 >= 0)
//            ips200_draw_point((F.feature_p_L[i].y-1)*158/188,78-(imgRow-1)+F.feature_p_L[i].x,RGB565_RED);
    }

    for(int i = 0;i<F.FP_n_R;i++){
//        ips200_draw_point(F.feature_p_R[i].y*158/188,78-(imgRow-1)+F.feature_p_R[i].x,RGB565_RED);
//        if(78-(imgRow-1)+F.feature_p_R[i].x+1 < 128)
//            ips200_draw_point(F.feature_p_R[i].y*158/188,78-(imgRow-1)+F.feature_p_R[i].x+1,RGB565_RED);
//        if(78-(imgRow-1)+F.feature_p_R[i].x-1 >= 0)
//            ips200_draw_point(F.feature_p_R[i].y*158/188,78-(imgRow-1)+F.feature_p_R[i].x-1,RGB565_RED);
//        if((F.feature_p_R[i].y+1)*158/188 < 160)
//            ips200_draw_point((F.feature_p_R[i].y+1)*158/188,78-(imgRow-1)+F.feature_p_R[i].x,RGB565_RED);
//        if((F.feature_p_R[i].y-1)*158/188 >= 0)
//            ips200_draw_point((F.feature_p_R[i].y-1)*158/188,78-(imgRow-1)+F.feature_p_R[i].x,RGB565_RED);
    }
}

/**
 * @brief 将边界分割函数，将边界分割为
 *          LorR 1---L 0---R
 * 
 * @param broder 目标边界
 * @param LorR   左右边界 因为缺线的值不同
 */
void Vision_GetSegment(int16* broder,uint8_t LorR)
{
    segment_t* target_segment = LorR? F.my_segment_L:F.my_segment_R;
    uint8_t begin_flag = 1;     //序列记录开始标志位
    int segment_n = 0;          //用于记录第几段序列

    //清空上一次的数据 clear
    if(LorR)
        F.segment_n_L = 0;
    else
        F.segment_n_R = 0;

    for(int i = 0;i<10;i++)
    {
        target_segment[i].type = NULL_segment;
        target_segment[i].begin = 0;
        target_segment[i].end = 0;
        target_segment[i].position = NEAR;
    }
    //将边界分段 并且标记
    for(int i = imgRow-1 ; i>0 ; i--)
    {
        //对第一个点进行处理
        if(begin_flag){
            target_segment[segment_n].begin = i == imgRow-1? i:i+1;
           
           //标定结果
           //丢线判定：三个像素均判定为丢线则认为为丢线
           //不是丢线则标记为未定位
            if(LorR){ //左边
                if((i-2>=0)&&broder[i]<=3&&broder[i-1]<=3){
                    target_segment[segment_n].type = lose_segment;
                    broder[i] = LEFT_LOSE_VALUE;
                    broder[i-1] = LEFT_LOSE_VALUE;
                    broder[i-2] = LEFT_LOSE_VALUE;
                }
                else
                    target_segment[segment_n].type = NULL_segment;
            }
            else if((!LorR)){//右边
                if((i-2>=0)&&broder[i]>=RIGHT_LOSE_VALUE-5&&broder[i-1]>=RIGHT_LOSE_VALUE-5){
                    target_segment[segment_n].type = lose_segment;
                    broder[i] = RIGHT_LOSE_VALUE;
                    broder[i-1] = RIGHT_LOSE_VALUE;
                    broder[i-2] = RIGHT_LOSE_VALUE;
                }
                else
                    target_segment[segment_n].type = NULL_segment;
            }
            begin_flag = 0;
        }

        //
        //记录结尾
        //
        if(target_segment[segment_n].type == lose_segment){
            //对于最后几个点不做判断
            if(i>=3){
                //防止突变 连续三个量都离边界较远时认为是lose
                if((LorR && broder[i]>5 && broder[i-1]>5 && broder[i-2]>5 && broder[i-3]>5)||    //左边
                    ((!LorR) && broder[i]<RIGHT_LOSE_VALUE-5 && broder[i-1]<RIGHT_LOSE_VALUE-5 && broder[i-2]<RIGHT_LOSE_VALUE-5 && broder[i-3]<RIGHT_LOSE_VALUE-5)){ //右边
                    target_segment[segment_n].end = i+1;//记录结尾
                    segment_n++;
                    begin_flag = 1;
                }
                else 
                    broder[i] = (LorR)? LEFT_LOSE_VALUE:RIGHT_LOSE_VALUE;
            }
            else
            ;
        }
        //若第一次检测到的不为丢失，则丢线后记录为下一段开始
        else if(target_segment[segment_n].type == NULL_segment){
            if(i>=2){
                //防止突变 连续三个量很小认为是lose
                if((LorR && broder[i]<=5 && broder[i-1]<=5 && broder[i-2]<=5)||    //左边
                    ((!LorR) && broder[i]>=RIGHT_LOSE_VALUE-5 && broder[i-1]>=RIGHT_LOSE_VALUE-5 && broder[i-2]>=RIGHT_LOSE_VALUE-5)){ //右边
                    broder[i] = LorR? LEFT_LOSE_VALUE:RIGHT_LOSE_VALUE;
                    broder[i-1] = LorR? LEFT_LOSE_VALUE:RIGHT_LOSE_VALUE;
                    broder[i-2] = LorR? LEFT_LOSE_VALUE:RIGHT_LOSE_VALUE;
                    target_segment[segment_n].end = i+1;//记录结尾
                    segment_n++;
                    begin_flag = 1;
                }
                else 
                    broder[i] = (broder[i]<=LEFT_LOSE_VALUE+5||broder[i]>=RIGHT_LOSE_VALUE - 5)? broder[i+1]:broder[i];
            }
            else
            ;
        }
    }

    // 当第一个序列太短时，与第二个序列合并
    if(target_segment[0].begin - target_segment[0].end <= 5){
        for(int i = target_segment[0].begin;i>=target_segment[0].end;i--){
            broder[i] = broder[target_segment[1].end];
        }

        target_segment[0].end = target_segment[1].end;
		target_segment[0].type = target_segment[1].type;
        for(int i = 1 ;i<=segment_n;i++)
        {
            target_segment[i].begin = target_segment[i+1].begin;
            target_segment[i].end = target_segment[i+1].end;
            target_segment[i].type = target_segment[i+1].type;
        }
        segment_n -= 1;
    }


    //记录数量
    if(LorR)
        F.segment_n_L = segment_n+1;
    else
        F.segment_n_R = segment_n+1;

    //对于边界进行判断
    for(int i = 0;i<segment_n+1;i++)
    {
       
        if(target_segment[i].type == NULL_segment){
             //是否为直线
            int straight = Line_IsStraight(broder,target_segment[i].begin,target_segment[i].end);
            if(straight){
                target_segment[i].type = straight_segment;
            }
            //根据单调性来对弯道以及圆弧进行判定
            else{
                int mono;
                mono = Line_IsMonotonous(broder,target_segment[i].begin,target_segment[i].end);
                //函数单调则为弯道，函数不单调则为圆弧
                target_segment[i].type = mono? corner_segment : arc_segment;
            }
        }
    }


}

/**
 * @brief 寻找一段弧的第一个特征点，方向由大到小
 * 
 * @param broder 边界
 * @param x1    上域
 * @param x2    下区域
 * @return point_t 返回第一个特征点的坐标 若返回值为-1，则没有发现特征点
 */
point_t Vision_FindArcFP(int16 *broder,int x1,int x2)
{
    int max = Tool_CmpMax(x1,x2);
    int min = Tool_CmpMin(x1,x2);

    int how_many = max - min + 1;
    int final_x = max - 1;

    int p_distance = 5;//检测区域
    float p_th = -0.3;//判定为角点的阈值
    float cosvalue;

    if(how_many >= p_distance + 2){
        cosvalue = Vector_AngleGet((point_t){max,broder[max]},
                                (point_t){max-1,broder[max-1]},
                                (point_t){max - p_distance,broder[max - p_distance]});
    }
    else{
        cosvalue = Vector_AngleGet((point_t){max,broder[max]},
                                (point_t){max-1,broder[max-1]},
                                (point_t){min,broder[min]});
    }

    //边界的点不考虑
    for(int i = max - 2; i>min+1; i--)
    {
        int low_x = (i-min)>=5?(i - 5):min;
        int high_x = (max - i)>=5?(i + 5):max;

        //滤去突变量
        if(broder[i] == LEFT_LOSE_VALUE|| broder[i] == RIGHT_LOSE_VALUE)
            broder[i] = broder[i-1];

        float currentcos = Vector_AngleGet((point_t){low_x,broder[low_x]},
                                            (point_t){i,broder[i]},
                                            (point_t){high_x,broder[high_x]});
        
        if(currentcos > cosvalue){
            final_x = i;
            cosvalue = currentcos;
        }
                                           
    }
    //找到特征点
    if(cosvalue > p_th)
        return (point_t){final_x,broder[final_x]};
    //未找到特征点
    else
        return (point_t){-1,-1};
}

/**
 * @brief 寻找边界特征点
 * 
 * @param broder 
 */
void Vision_BroderFindFP(int16* broder)
{
    segment_t *target_seg;
    int* target_n;
    point_t* target_FP;
    int lose_value;
    int segment_n;

    if(broder == Image_S.leftBroder){
        target_seg = F.my_segment_L;
        target_n = &(F.FP_n_L);
        target_FP = F.feature_p_L;
        segment_n = F.segment_n_L;
        lose_value = LEFT_LOSE_VALUE;
    }
    else if(broder == Image_S.rightBroder){
        target_seg = F.my_segment_R;
        target_n = &(F.FP_n_R);
        target_FP = F.feature_p_R;
        segment_n = F.segment_n_R;
        lose_value = RIGHT_LOSE_VALUE;
    }

    //清空数据
    (*target_n) = 0;
    for(int i = 0;i<3;i++){
        target_FP[i].x = 0;
        target_FP[i].y = 0;
    }

    /*---------------角点检测---------------*/
    int aver1;
    int aver2;
    point_t pf;
    //中间缺线 且缺线达到阈值
    if(IsLose(target_seg[1])&&Vision_IsLone(target_seg[1])){
        aver1 = Line_GetAverage(broder,target_seg[0].begin,target_seg[0].end);
        
        //检测近处序列的角点
        pf = Vision_FindArcFP(broder,target_seg[0].begin,target_seg[0].end);
        //若没有发现角点
        if((pf.x == -1 && pf.y == -1)){
            target_FP[0].x = target_seg[0].end;
            target_FP[0].y = broder[target_seg[0].end];
            (*target_n)++;
            //计算近处序列的平均斜率 用于预防u形弯误判
        
            //*检测远处序列的角点  当远处序列有值的时候检测 有值的时候必定有角点*
            if(!IsNull(target_seg[2])){
                //若远处序列和近处序列差别太大，认为为突变
                aver2 = Line_GetAverage(broder,target_seg[2].begin,target_seg[2].end);
                if(fabs(aver1 - aver2)>80){
                    target_FP[0].x = -1;
                    target_FP[0].y = -1;
                    (*target_n)--;
                    return;
                }
                //只寻找较近序列的特征点
                pf = (target_seg[2].begin - target_seg[2].end >= 15)?
                    Vision_FindArcFP(broder,target_seg[2].begin,target_seg[2].begin - 15) :
                    Vision_FindArcFP(broder,target_seg[2].begin,target_seg[2].end);
                    //*存在角点记录角点，不存在记录边界点*
                target_FP[1].x = (pf.x == -1) ? target_seg[2].begin : pf.x ;
                target_FP[1].y = (pf.y == -1) ? broder[target_seg[2].begin] : pf.y;
                (*target_n)++;
            }

        }//发现了角点
        else{
            target_FP[0] = pf;
            (*target_n)++;

           //*检测远处序列的角点  当远处序列有值的时候检测 有值的时候必定有角点*
          if(!IsNull(target_seg[2])){
                //若远处序列和近处序列差别太大，认为为突变
                aver2 = Line_GetAverage(broder,target_seg[2].begin,target_seg[2].end);
                if(fabs(aver1 - aver2)>80){
                    target_FP[0].x = -1;
                    target_FP[0].y = -1;
                    (*target_n)--;
                    return;
                }
                //只寻找较近序列的特征点
                pf = (target_seg[2].begin - target_seg[2].end >= 15)?
                    Vision_FindArcFP(broder,target_seg[2].begin,target_seg[2].begin - 15) :
                    Vision_FindArcFP(broder,target_seg[2].begin,target_seg[2].end);
                //*存在角点记录角点，不存在记录边界点*
                target_FP[1].x = (pf.x == -1) ? target_seg[2].begin : pf.x ;
                target_FP[1].y = (pf.y == -1) ? broder[target_seg[2].begin] : pf.y;
                (*target_n)++;
            }
        }
    }

    //开头缺线
    else if(target_seg[0].type == lose_segment){
        //检测远处序列的角点  当远处序列有值的时候检测
        if(!IsNull(target_seg[1])){
            pf = (target_seg[1].begin - target_seg[1].end >= 15)?
                Vision_FindArcFP(broder,target_seg[1].begin,target_seg[1].begin - 15) :
                Vision_FindArcFP(broder,target_seg[1].begin,target_seg[1].end);
            //当未发现角点
            target_FP[0].x = (pf.x == -1) ? target_seg[1].begin : pf.x ;
            target_FP[0].y = (pf.y == -1) ? broder[target_seg[1].begin] : pf.y;
            (*target_n)++;
        }
    }
    //斜入十字特殊情况（十字接环岛） 只寻找近处的角点
    else if(IsArc(target_seg[0])&&segment_n == 1){
        pf = Vision_FindArcFP(broder,target_seg[0].begin,25);
        if(pf.x != -1&&pf.y != -1){
					target_FP[0] = pf;
					(*target_n)++;
        }
    }
}

/**
 * @brief 判断前方是否有斑马线
 * 
 * @return uint8_t 返回1为发现前方斑马线
 */
#define Zebra_Y 60
#define Zebra_TH 10
#define Zebra_Range 10
uint8_t Vision_IsZebra(){
    int change_num = 0;
    int mid = (Image_S.leftBroder[30]+Image_S.rightBroder[30])/2;

    for(int i = mid;i<140;i++){
        if(my_image[Zebra_Y][i] > Threshold+Zebra_Range && my_image[Zebra_Y][i+1] < Threshold-Zebra_Range||
            my_image[Zebra_Y][i] < Threshold-Zebra_Range && my_image[Zebra_Y][i+1] > Threshold+Zebra_Range){
            change_num++;
        }
    }

    for(int i = mid;i>46;i--){
        if(my_image[Zebra_Y][i] > Threshold+Zebra_Range && my_image[Zebra_Y][i+1] < Threshold-Zebra_Range||
            my_image[Zebra_Y][i] < Threshold-Zebra_Range && my_image[Zebra_Y][i+1] > Threshold+Zebra_Range){
            change_num++;
        }
    }

    if(change_num >= Zebra_TH)
        return 1;

    return 0;
    
}

/**
 * @brief 打印边界数组
 * 
 */
void Vision_BroderPrint()
{
    rt_kprintf("left:\n");
    for(int i = 0; i< imgRow-1; i++){
        rt_kprintf("%d\n",Image_S.leftBroder[i]);
    }
    rt_kprintf("right:\n");
    for(int i = 0; i< imgRow-1; i++){
        rt_kprintf("%d\n",Image_S.rightBroder[i]);
    }
}

/**
 * @brief 意外情况打印信息
 * 
 */
void Vision_ErrorLogin()
{
    rt_kprintf("Judge Error\n");
    rt_kprintf("left ==> seg_num:%d ",F.segment_n_L);
    for(int i = 0;i<F.segment_n_L;i++){
        char s[128];
        switch (F.my_segment_L[i].type)
        {
        case NULL_segment:
            strcpy(s,"NULL_segment");
            break;

        case straight_segment:
            strcpy(s,"straight_segment");
            break;

        case lose_segment:
            strcpy(s,"lose_segment");
            break;

        case arc_segment:
            strcpy(s,"arc_segment");
            break;
        
        case corner_segment:
            strcpy(s,"corner_segment");
            break;

        default:
            strcpy(s,"error_type");
            rt_kprintf("(%d)",F.my_segment_L[i].type);
            break;
        }
        rt_kprintf("segment%d_type:%s ",i,s);
    }

    rt_kprintf("\nright ==> seg_num:%d ",F.segment_n_R);

    for(int i = 0;i<F.segment_n_R;i++){
        char s[128];
        switch (F.my_segment_R[i].type)
        {
        case NULL_segment:
            strcpy(s,"NULL_segment");
            break;

        case straight_segment:
            strcpy(s,"straight_segment");
            break;

        case lose_segment:
            strcpy(s,"lose_segment");
            break;

        case arc_segment:
            strcpy(s,"arc_segment");
            break;
        
        case corner_segment:
            strcpy(s,"corner_segment");
            break;

        default:
            strcpy(s,"error_type");
            rt_kprintf("(%d)",F.my_segment_L[i].type);
            break;
        }
        rt_kprintf("segment%d_type:%s ",i,s);
    }
		rt_kprintf("\n");
}

/**
 * @brief 延长线段到边界 但是固定K值
 *          direction --- 补线方向
 *          1---向远处补线 0---向近处补线
 * 
 * @param broder 边界数组
 * @param x 起始点
 * @param direction 补线方向
 * @param slope 斜率
 */
#define FAR_Extend 1
#define NEAR_Extend 0
void Vision_ExtendLineK(int16 *broder,int x,int direction,float slope){
    //向远处补线
    if(direction){
        for(int i = x-1 ; i > 0 ; i--)
        {
            broder[i] = broder[x] - slope * (x - i);

            broder[i] = broder[i]<0? 0:broder[i];
            broder[i] = broder[i]>159?159:broder[i];
            
            broder[i] = broder[i]>imgCol-1? imgCol-1:broder[i];
        }
    }
    //向近处补线
    else{
        for(int i = 0 ; i < imgRow-1-x ; i++)
        {
            broder[x+i] = broder[x] + slope * i;

            broder[x+i] = broder[x+i]<0? 0:broder[x+i];
            broder[x+i] = broder[x+i]>159? 159:broder[x+i];

            broder[x+i] = broder[x+i]>imgCol-1? imgCol-1:broder[x+i];
        }
    }

}

/**
 * @brief 计算边界缺陷度函数
 * 
 * @param broder 对应边界
 * @return float 缺陷率
 */
float Vision_GetLoseRate(int16* broder){
    int LorR = (broder == Image_S.leftBroder)?1:0;
    int lose_num;
    for(int i = 0;i<IMAGE_ROW;i++)
    {
        lose_num = (LorR&&broder[i]==LEFT_LOSE_VALUE)||(!LorR&&broder[i]==RIGHT_LOSE_VALUE)? 
                    lose_num+1:lose_num;
    }
    return (float)lose_num/IMAGE_ROW;
}

/**
 * @brief 延长线段到边界
*   direction --- 补线方向
*   1---向远处补线 0---向近处补线
*/
void Vision_ExtendLine(int16 *broder,int x,int direction)
{   
    float this_K;
    //向远处补线
    if(direction){

        this_K = (x+5<=imgRow-1)?(float)(broder[x + 5] - broder[x])/5.0f:(float)(broder[imgRow-1] - broder[x])/(float)(imgRow - 1 - x);
        for(int i = x-1 ; i > 0 ; i--)
        {
            broder[i] = broder[x] - this_K * (x - i);
            broder[i] = broder[i]<0? 0:broder[i];
            broder[i] = broder[i]>imgCol-1? imgCol-1:broder[i];
        }
    }
    //向近处补线
    else{
        this_K = (x-5 >= 0)?(float)(broder[x] - broder[x - 5])/5.0f:(float)(broder[x] - broder[0])/(float)x;
        for(int i = 0 ; i < imgRow-1-x ; i++)
        {
            broder[x+i] = broder[x] + this_K * i;
            broder[x+i] = broder[x+i]<0? 0:broder[x+i];
            broder[x+i] = broder[x+i]>imgCol-1? imgCol-1:broder[x+i];
        }
    }

}

/**
 * @brief 补线函数，告诉两个点的数组下表，将对应边界两点间的值补充为直线
 * 
 * @param p1 横坐标1
 * @param p2 横坐标2
 * @param broder 边界
 */
void Vision_set_AdditionalLine(int16 p1,int16 p2,int16 *broder)
{
		float slope = Point_CalSlope((point_t){p1,broder[p1]},(point_t){p2,broder[p2]});
		
		int pmin = p1>p2? p2:p1;
		int pmax = p1>p2? p1:p2;
		
		for(int i = pmin ; i<=pmax ; i++)
		{
			broder[i] = (int16)((float)(i-pmin)*slope + (float)broder[pmin]);
		}
}

/**
 * @brief 将边界对应段置于缺陷状态
 * 
 * @param broder 边界
 * @param x1 段首
 * @param x2 段尾
 */
void Vision_SetLose(int16* broder,int16 x1,int16 x2){
    int max = Tool_CmpMax(x1,x2);
    int min = Tool_CmpMin(x1,x2);

    for(int i = min;i<max;i++){
        broder[i] = (broder == Image_S.leftBroder)? LEFT_LOSE_VALUE:RIGHT_LOSE_VALUE;
    }
}

/**
 * @brief 基于一个点和一个固定的斜率画直线
 * 
 * @param broder 边界
 * @param seed   某一点坐标坐标
 * @param slope  斜率
 * @param x1     直线范围1
 * @param x2     直线范围2
*/
void Vision_SetLineWithPointK(int16* broder,int seed,float slope,int x1,int x2){
    int max = Tool_CmpMax(x1,x2);
    int min = Tool_CmpMin(x1,x2);

    for(int i = min; i<=max ;i++){
        broder[i] = broder[seed] + (i - seed)*slope;
    }
}

/**
 * @brief 十字赛道处理
 * 
 */
#define Cross_Begin   0
#define Cross_State_1 1
uint8_t L_or_R_Cross;
void Vision_CrossHandle()
{
    static int state = Cross_Begin;

        //状态切换
    if(state == Cross_Begin){
        //当进入十字的时候发现两边都未能发现角点 退出十字
        //十字的时候应当有明显的角点
        // rt_kprintf("RS:Cross\n");
        if(F.FP_n_L == 0&&F.FP_n_R == 0){
            Current_Road = NormalRoads;
            return;
        }
        if(IsLose(F.my_segment_L[0])&&IsLose(F.my_segment_R[0])){
             state = Cross_State_1;
             //BUZZER_SPEAK;
        }
    }
    else if(state == Cross_State_1){
        if(!IsLose(!F.my_segment_L[0])&&!IsLose(F.my_segment_R[0])){
            //BUZZER_SPEAK;
            if(Start_Flag){
                Car_Change_Speed(0,0,0);
                point_t first_point = {0,(Image_S.leftBroder[0]+Image_S.rightBroder[0])/2};
                point_t last_point = {30,(Image_S.leftBroder[30]+Image_S.rightBroder[30])/2};
                float k = Line_CalK(first_point,last_point);
                rt_kprintf("%d,%d,%.2f\n",(Image_S.leftBroder[30]+Image_S.rightBroder[30])/2,(Image_S.leftBroder[0]+Image_S.rightBroder[0])/2,k);
                // for(int i = 0;i<imgRow;i++)
                //     rt_kprintf("%d\n",Image_S.leftBroder[i]);
                //  for(int i = 0;i<imgRow;i++)
                //     rt_kprintf("%d\n",Image_S.rightBroder[i]);
                // while(1);
                if(k<0){
                    L_or_R_Cross = RIGHT_CROSS;
                }
                else 
                    L_or_R_Cross = LEFT_CROSS;
            }
            state = Cross_Begin;
            Current_Road = NormalRoads;
            
        }
    }
    //补线
    //斜入十字
    if(CrossCon4){
        //左边
        if (IsArc(F.my_segment_L[0])&&F.segment_n_L == 1){
            //此时应当左边找到一个特征点 计算特征点到另一边角点的斜率
            Vision_ExtendLine(Image_S.leftBroder,F.feature_p_L[0].x,1);
            Vision_ExtendLine(Image_S.rightBroder,F.feature_p_R[0].x,0);
        }
        //右边
        else if(IsArc(F.my_segment_R[0])&&F.segment_n_R == 1){
            Vision_ExtendLine(Image_S.leftBroder,F.feature_p_L[0].x,0);
            Vision_ExtendLine(Image_S.rightBroder,F.feature_p_R[0].x,1);
        }
    }
    // else{
    //     if(F.FP_n_L == 2)
    //         Vision_set_AdditionalLine(F.feature_p_L[0].x,F.feature_p_L[1].x,Image_S.leftBroder);
    //     else if(F.FP_n_L == 1){
    //         if(IsLose(F.my_segment_L[0]))
    //             Vision_ExtendLine(Image_S.leftBroder,F.feature_p_L[0].x,0);
    //         else
    //             Vision_ExtendLine(Image_S.leftBroder,F.feature_p_L[0].x,1);
    //     }

    //     if(F.FP_n_R == 2)
    //         Vision_set_AdditionalLine(F.feature_p_R[0].x,F.feature_p_R[1].x,Image_S.rightBroder);
    //     else if(F.FP_n_R == 1){
    //         if(IsLose(F.my_segment_R[0]))
    //             Vision_ExtendLine(Image_S.rightBroder,F.feature_p_R[0].x,0);
    //         else
    //             Vision_ExtendLine(Image_S.rightBroder,F.feature_p_R[0].x,1);
    //     }
    // }
   
}

/************************************************************************************************************************************************
 * @brief 弯道处理
 * 
 ***********************************************************************************************************************************************/
void Vision_CornerHandle()
{
    if(CornerState1){
        //暂时认为如果两边都是1的话不需要处理
        //就算单边全丢线 ，但是另一边仍然能够使得中线偏离
    }
    else if(CornerState2){
        if(F.my_segment_L[1].end - F.my_segment_L[1].begin > F.my_segment_R[1].end - F.my_segment_R[1].begin){
            //右边缺的少 左转 
            for(int i = F.my_segment_R[1].begin;i>F.my_segment_R[1].end;i--){
                Image_S.rightBroder[i] = LEFT_LOSE_VALUE;
            }
        }
        else if(F.my_segment_L[1].end - F.my_segment_L[1].begin < F.my_segment_R[1].end - F.my_segment_R[1].begin){
            //左边缺的少 右边转 
            for(int i = F.my_segment_L[1].begin;i>F.my_segment_L[1].end;i--){
                Image_S.leftBroder[i] = LEFT_LOSE_VALUE;
            }
        }
    }
    else if(CornerState3){
        //左边缺线 将右边部分向左移
        if(IsLose(F.my_segment_L[0]))
            for(int i = F.my_segment_R[1].begin;i>F.my_segment_R[1].end;i--){
                Image_S.rightBroder[i] = LEFT_LOSE_VALUE;
            }
        //右边缺线 将左边部分向右移
        else if(IsLose(F.my_segment_R[0]))
            for(int i = F.my_segment_L[1].begin;i>F.my_segment_L[1].end;i--){
                Image_S.leftBroder[i] = LEFT_LOSE_VALUE;
            }
    }

    Current_Road = NormalRoads;
}

/**************************************************************************************************************************************************
 * @brief 圆环处理
 * 
 *************************************************************************************************************************************************/

#define Circule_Begin  0
#define Circule_State1 1
#define Circule_State2 2
#define Circule_State3 3
#define Circule_in     4
#define Circule_Cor    5
#define Circule_out    6
#define Circule_end    7

struct {
    segment_t*  circule_seg;
    segment_t*  anti_cir_seg;

    int16_t*    circule_broder;
    int16_t*    anti_cir_broder;

    point_t *   circule_fp;
    point_t *   anti_cir_fp;

    int*        circule_fp_n;
    int*        anti_cir_fp_n;

    int*        circule_seg_n;
    int*        anti_cir_seg_n;

    uint8_t     tick;           //滤波器
    uint8_t     state;          //圆环状态
    uint8_t     Circule_LorR;   //左右圆环标志

}Circule_Handle;

/**
 * @brief 圆环状态1补充线
 *          近端角点向远端画线  
 * 
 */
void Vision_CirculeState1_Handle(){
    
    static int tick = 0;

    if(*(Circule_Handle.circule_fp_n) && !IsLose(Circule_Handle.circule_seg[0])){
        float slope = Point_CalSlope((point_t){0,Circule_Handle.anti_cir_broder[0]},(point_t){imgRow-1,Circule_Handle.anti_cir_broder[imgRow-1]});
        //负的斜率因为没做透视变换
        Vision_SetLineWithPointK(Circule_Handle.circule_broder,Circule_Handle.circule_fp[0].x,-slope,0,imgRow-1);
    }
    if(IsLose(Circule_Handle.circule_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick == 5){
            Circule_Handle.state = Circule_State2; 
            rt_kprintf("RS:Cir State2\n");
            Circule_Handle.tick = 0;
        }
    }
}

/**
 * @brief 圆环状态2补充线
 *          远端圆弧最小/大点画线
 * 
 */
void Vision_CirculeState2_Handle(){

    if(IsLose(Circule_Handle.circule_seg[0])){
        //计算直线的平均斜率
        float slope = Point_CalSlope((point_t){0,Circule_Handle.anti_cir_broder[0]},(point_t){imgRow-1,Circule_Handle.anti_cir_broder[imgRow-1]});
        //得到圆弧的最小点
        int seed = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?Line_FindMaxPoint(Circule_Handle.circule_broder,Circule_Handle.circule_seg[1].begin,Circule_Handle.circule_seg[1].end):Line_FindMinPoint(Circule_Handle.circule_broder,Circule_Handle.circule_seg[1].begin,Circule_Handle.circule_seg[1].end);
        //负的斜率因为没做透视变换
        Vision_SetLineWithPointK(Circule_Handle.circule_broder,seed,-slope,0,imgRow-1);
    }
        
    else if(!IsLose(Circule_Handle.circule_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick == 1){
            rt_kprintf("RS:Cir State3\n");
            Circule_Handle.state = Circule_State3;
            Circule_Handle.tick = 0;
        }
    }

}

/**
 * @brief 圆环状态3处理
 *       近处圆弧最小/大点  弯道最近点 画线
 *       
 */
void Vision_CirculeState3_Handle()
{
     if(!IsLose(Circule_Handle.circule_seg[0])){
        //计算直线的平均斜率
        float slope = Point_CalSlope((point_t){0,Circule_Handle.anti_cir_broder[0]},(point_t){imgRow-1,Circule_Handle.anti_cir_broder[imgRow-1]});
        //根据远处的角点进行补线
        if(IsArc(Circule_Handle.circule_seg[0])){
            int seed = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?Line_FindMaxPoint(Circule_Handle.circule_broder,Circule_Handle.circule_seg[0].begin,Circule_Handle.circule_seg[0].end):Line_FindMinPoint(Circule_Handle.circule_broder,Circule_Handle.circule_seg[0].begin,Circule_Handle.circule_seg[0].end);
            Vision_SetLineWithPointK(Circule_Handle.circule_broder,seed,-slope,0,imgRow-1);
        }
        else if(IsCorner(Circule_Handle.circule_seg[0]))
            Vision_SetLineWithPointK(Circule_Handle.circule_broder,Circule_Handle.circule_seg[0].begin,-slope,0,imgRow-1);
    }
    else if(IsLose(Circule_Handle.circule_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick == 2){
            Circule_Handle.state = Circule_in;
            rt_kprintf("RS:Cir in\n");
            Circule_Handle.tick = 0;
        }
    }
}


/**
 * @brief 圆环入环处理
 *      直接从左边向右边画一条直线
 *      如果发现一边是弧形或者弯道，进入环内
 */
void Vision_CirculeIn_Handle(){
     //直接从左向右画一条直线
    Circule_Handle.anti_cir_broder[0] = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)? 15:187;
    Circule_Handle.anti_cir_broder[69] = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)? 187:20;
    Vision_set_AdditionalLine(0,imgRow-1,Circule_Handle.anti_cir_broder); 
    Vision_SetLose(Circule_Handle.circule_broder,0,imgRow-1);

    if(IsArcCorner(Circule_Handle.anti_cir_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick==4){
            Circule_Handle.state = Circule_Cor;
            Circule_Handle.tick = 0;
            rt_kprintf("RS:Cir State5\n");
        }
    }
}
/**
 * @brief 圆环环内 不作任何处理
 *        检测到圆弧角后，进入圆环出环处理
 * 
 */
void Vision_CirculeCor_Handle(){
    if(IsArc(Circule_Handle.anti_cir_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick==5){
            Circule_Handle.state = Circule_out;
            rt_kprintf("RS:Cir Out\n");
            Circule_Handle.tick = 0;
        }
    }
    else
        Circule_Handle.tick = 0;
}

/**
 * @brief 圆环出环处理
 * 
 */
void Vision_CirculeOut_Handle(){

    Circule_Handle.anti_cir_broder[0] = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)? 7:180;
    Circule_Handle.anti_cir_broder[69] = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)? 180:10;

    Vision_set_AdditionalLine(0,69,Circule_Handle.anti_cir_broder);       
    Vision_SetLose(Circule_Handle.circule_broder,0,69);

    if(Circule_Handle.anti_cir_fp_n){
        int i = 0;
        float slope = Point_CalSlope(Circule_Handle.anti_cir_fp[0],(point_t){Circule_Handle.anti_cir_fp[0].x + 3,Circule_Handle.anti_cir_broder[Circule_Handle.anti_cir_fp[0].x + 3]});
        while((Circule_Handle.Circule_LorR?(slope>0):(slope<0))){
            slope = Point_CalSlope( (point_t){Circule_Handle.anti_cir_fp[0].x + i       ,Circule_Handle.anti_cir_broder[Circule_Handle.anti_cir_fp[0].x + i]},
                                    (point_t){Circule_Handle.anti_cir_fp[0].x + i + 3   ,Circule_Handle.anti_cir_broder[Circule_Handle.anti_cir_fp[0].x + i + 3]});
            
        }

        Vision_SetLineWithPointK(Circule_Handle.anti_cir_broder,Circule_Handle.anti_cir_fp[0].x,slope,Circule_Handle.anti_cir_fp[0].x + i,0);
    }
    
    if(IsStrai(Circule_Handle.anti_cir_seg[0])){
        Circule_Handle.tick++;
        if(Circule_Handle.tick==2){
            Circule_Handle.state = Circule_end;
            rt_kprintf("RS:Cir end\n");
            Circule_Handle.tick = 0;
        }
    }
}

void Vision_CirculeEnd_Handle(){

    if(IsLose(Circule_Handle.circule_seg[0]) && !IsLose(Circule_Handle.circule_seg[1]) && Circule_Handle.circule_fp_n){
        //计算直线的平均斜率
        float slope = Line_GetAverK(Circule_Handle.circule_broder,Circule_Handle.circule_fp[0].x,Circule_Handle.circule_seg[1].end);
        Vision_SetLineWithPointK(Circule_Handle.circule_broder,Circule_Handle.circule_fp[0].x,slope,0,imgRow-1);
    }

    if(!IsLose(Circule_Handle.circule_seg[0]) && *Circule_Handle.circule_seg_n == 1){
        Current_Road = NormalRoads;
        Circule_Handle.state = Circule_Begin;
        rt_kprintf("RS:out of cir\n");
        Car_Change_Speed(0,0,0);
        rt_thread_delay(200);
        Car_Stop();
    }

}

/**
 * @brief 常规圆环思路
 * 
 */
void Vision_Cir_PI_Handle(){

    switch (Circule_Handle.state)
    {
        case Circule_State1:
             Vision_CirculeState1_Handle();
        break;

        case Circule_State2:
            //Vision_CirculeState2_Handle();
            Car_Change_Speed(0,0,0);
            rt_thread_delay(200);
            Car_DistanceMotion(0,20,0.3);
            rt_kprintf("RS:Cir State3\n");
            Circule_Handle.state = Circule_State3;
            Circule_Handle.tick = 0;
        break;

        case Circule_State3:
            Car_Change_Speed(0,0,0);
            rt_thread_delay(100);
            Car_Rotate( (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?90:-90);
            rt_thread_delay(500);
            Car_Speed_ConRight = Con_By_TraceLine;
            Circule_Handle.state = Circule_Cor;
        break;

        case Circule_Cor:
            //Vision_CirculeCor_Handle();
            if(IsLose(Circule_Handle.circule_seg[0]) && IsLose(Circule_Handle.anti_cir_seg[0])){
                Circule_Handle.tick++;
                if(Circule_Handle.tick >= 2){
                    Circule_Handle.state = Circule_out;
                    rt_kprintf("RS:Cir Out\n");
                    Circule_Handle.tick = 0;
                }
            }
            else 
                Circule_Handle.tick = 0;
        break;

        case Circule_out:
            if(IsLose(Circule_Handle.anti_cir_seg[0])){
                Car_Change_Speed(0,0,0);
                rt_thread_delay(100);
                Car_DistanceMotion(0,10,0.3);
                Car_Rotate( (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?90:-90);
                rt_thread_delay(500);
                Car_Speed_ConRight = Con_By_TraceLine;
                Circule_Handle.state = Circule_Begin;
                Current_Road = NormalRoads;
            }
        break;
    }
}

/**
 * @brief PI入环处理
 * 
 */
void Vision_Cir_Normal_Handle(){

    switch (Circule_Handle.state)
    {
        case Circule_State1:
             Vision_CirculeState1_Handle();
        break;

        case Circule_State2:
            Vision_CirculeState2_Handle();
        break;

        case Circule_State3:
            Vision_CirculeState3_Handle();
        break;

        case Circule_in:
            Vision_CirculeIn_Handle();
        break;

        case Circule_Cor:
            Vision_CirculeCor_Handle();
        break;

        case Circule_out:
            Vision_CirculeOut_Handle();
        break;

        case Circule_end:
            Vision_CirculeEnd_Handle();
        break;
    }

}

void Vision_CirculeHandle()
{   
    if(!Circule_Handle.state){
        rt_kprintf("RS:Circule\n");

        if(IsStrai(F.my_segment_L[0]) && F.segment_n_L == 1)
            Circule_Handle.Circule_LorR = RIGHT_CIRCULE;
        else if(IsStrai(F.my_segment_R[0]) && F.segment_n_R == 1)
            Circule_Handle.Circule_LorR = LEFT_CIRCULE;

        Circule_Handle.circule_seg     = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?F.my_segment_L     :F.my_segment_R;
        Circule_Handle.anti_cir_seg    = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?F.my_segment_R     :F.my_segment_L;

        Circule_Handle.circule_broder  = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?Image_S.leftBroder :Image_S.rightBroder;
        Circule_Handle.anti_cir_broder = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?Image_S.rightBroder:Image_S.leftBroder;

        Circule_Handle.circule_fp      = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?F.feature_p_L      :F.feature_p_R;
        Circule_Handle.anti_cir_fp     = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?F.feature_p_R      :F.feature_p_L;

        Circule_Handle.circule_fp_n    = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?&F.FP_n_L          :&F.FP_n_R;
        Circule_Handle.anti_cir_fp_n   = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?&F.FP_n_R          :&F.FP_n_L;

        Circule_Handle.circule_seg_n   = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?&F.segment_n_L    :&F.segment_n_R;
        Circule_Handle.anti_cir_seg_n  = (Circule_Handle.Circule_LorR == LEFT_CIRCULE)?&F.segment_n_R    :&F.segment_n_L;

        Circule_Handle.tick            = 0;
        Circule_Handle.state           = Circule_State1;
    }


    //防止错误判断
    if(Circule_Handle.state == Circule_State1 || Circule_Handle.state == Circule_State2 || Circule_Handle.state == Circule_State3){

        static int out_n;
        out_n = (!IsStrai(Circule_Handle.anti_cir_seg[0])&&(Vision_GetSegLenghth(Circule_Handle.anti_cir_seg[0]) >= 20))? out_n+1:0;
        if(out_n == 3){
            Current_Road = NormalRoads;
            Circule_Handle.state = Circule_Begin;
            out_n = 0;
            rt_kprintf("RS:error detection\n");
        }
    }
    
    // Vision_Cir_Normal_Handle();
    Vision_Cir_PI_Handle();
}

/**
 * @brief 斑马线处理函数
 * 
 */
void Vision_ZebraHandle(){
    if(Start_Flag){
            Car_Change_Speed(0,0,0);
            rt_thread_delay(10);
            Car_DistanceMotion(0,50,1.5);
            rt_kprintf("Task Finished\n");
            while(1){
                rt_thread_delay(10);
            }
    }
    Current_Road = NormalRoads;

}

/**
 * @brief 计算黑点比例
 * 
 * @param img_bwp 灰度数组
 * @param x1 第一个x坐标
 * @param y1 第一个y坐标
 * @param x2 第二个x坐标
 * @param y2 第二个y坐标
 * @return float 黑色像素占比
 */
float Vision_CalBlackRate(uint8 *img_bwp,uint16_t width,uint16_t height,point_t p1,point_t p2){
    int black = 0;
    int max_x = Tool_CmpMax(p1.x,p2.x);
    int min_x = Tool_CmpMin(p1.x,p2.x);
    int max_y = Tool_CmpMax(p1.y,p2.y);
    int min_y = Tool_CmpMin(p1.y,p2.y);

    for(int i = min_x;i<=max_x;i++){
        for(int j = min_y;j<=max_y;j++){
            if(img_bwp[j*width+i] == 0)
                black++;
        }
    }

    int total = (max_x-min_x+1)*(max_y-min_y+1);
    return (float)black/total;

}

/**
 * @brief 计算长方形区域的左右对称性
 * 
 * @param img 图像指针
 * @param width 图像宽度
 * @param height 图像高度
 * @param p1 第一个点
 * @param p2 第二个点
 * @param ud_or_lr 1对应左右对称，0对应上下对称
 * @return float 
 */
float Vision_CalSymRate(uint8_t *img,uint16_t width,uint16_t height,point_t p1,point_t p2,uint8_t ud_or_lr){
    uint16_t max_x = Tool_CmpMax(p1.x,p2.x);
    uint16_t min_x = Tool_CmpMin(p1.x,p2.x);
    uint16_t max_y = Tool_CmpMax(p1.y,p2.y);
    uint16_t min_y = Tool_CmpMin(p1.y,p2.y);

    uint16_t sys_p_n = 0;
    uint16_t total_n = (max_x - min_x + 1)*(max_y - min_y + 1);

    for(int i = min_x;i<=max_x;i++) {
        for (int j = min_y; j <= max_y; j++) {
            if (ud_or_lr) {
                if (img[j * width + i] == img[j * width + width - i - 1])
                    sys_p_n++;
            }
            else {
                if (img[j * width + i] == img[(height - j - 1) * width + i])
                    sys_p_n++;
            }
        }
    }

    return (float)((float)sys_p_n/(float)total_n);
}