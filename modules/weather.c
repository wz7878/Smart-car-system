#include "weather.h"

char wether[100][100] = {
    "A:/home/wz/晴.bmp",
    "A:/home/wz/多云.bmp",
    "A:/home/wz/阴.bmp",
    "A:/home/wz/小雨.bmp",
    "A:/home/wz/中雨.bmp",
    "A:/home/wz/大雨.bmp",
    "A:/home/wz/晴d.bmp",
    "A:/home/wz/多云d.bmp",
    "A:/home/wz/阴d.bmp",
    "A:/home/wz/小雨d.bmp",
    "A:/home/wz/中雨d.bmp",
    "A:/home/wz/大雨d.bmp"};

char week[100][50] = {
    "星期一",
    "星期二",
    "星期三",
    "星期四",
    "星期五",
    "星期六",
    "星期日"};

void choice_weather(lv_obj_t *img, char *typ)
{
    if(strcmp(typ,"晴")==0)
    {
        lv_image_set_src(img, wether[0]);
    }
    else if(strcmp(typ,"多云")==0)
    {
        lv_image_set_src(img, wether[1]);
    }
    else if(strcmp(typ,"阴")==0)
    {
        lv_image_set_src(img, wether[2]);
    }
    else if(strcmp(typ,"小雨")==0)
    {
        lv_image_set_src(img, wether[3]);
    }
    else if(strcmp(typ,"中雨")==0)
    {
        lv_image_set_src(img, wether[4]);
    }
    else if(strcmp(typ,"大雨")==0)
    {
        lv_image_set_src(img, wether[5]);
    }
}

void choice_week(lv_obj_t *label, char *wk,
                lv_obj_t *tmph ,char *temh,
                lv_obj_t *tmpl, char *teml)
{
    if(strcmp(wk,"星期一")==0)
    {
        lv_label_set_text(label, week[0]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期二")==0)
    {
        lv_label_set_text(label, week[1]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期三")==0)
    {
        lv_label_set_text(label, week[2]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期四")==0)
    {
        lv_label_set_text(label, week[3]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期五")==0)
    {
        lv_label_set_text(label, week[4]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期六")==0)
    {
        lv_label_set_text(label, week[5]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
    else if(strcmp(wk,"星期日")==0)
    {
        lv_label_set_text(label, week[6]);
        lv_label_set_text(tmph, temh);
        lv_label_set_text(tmpl, teml);
    }
}

void *http_get_weather(char *arg)
{
    int weth_sock=socket( AF_INET,SOCK_STREAM,0 );

    struct sockaddr_in weth_addr;
    weth_addr.sin_family=AF_INET;
    weth_addr.sin_port=htons(80);
    weth_addr.sin_addr.s_addr=inet_addr("175.6.241.203");

    int ret=connect(weth_sock,(struct sockaddr*)&weth_addr,sizeof(weth_addr));
    if(ret<0)
    {
        perror("connect");
        return -1;
    }
    else
    {
        printf("connect success\n");
    }

    char *http_request="GET /api/weather/city/101280101 HTTP/1.1\r\nHost:t.weather.itboy.net\r\n\r\n";

    write(weth_sock,http_request,strlen(http_request));

    char buf[8192]={0};
    read(weth_sock,buf,sizeof(buf));

    // 6. 提取JSON响应体（跳过HTTP响应头）
    char* json_start = strstr(buf, "\r\n\r\n");
    if (json_start == NULL) {
        printf("无法找到JSON响应体\n");
        close(weth_sock);
        return -1;
    }
    json_start += 4;  // 跳过"\r\n\r\n"

    //解析json数据
    cJSON *root = cJSON_Parse(json_start);
    if (root == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            printf("cJSON_Parse fail: 在 %s 处发现错误\n", error_ptr);
        } else {
            printf("cJSON_Parse fail\n");
        }
        close(weth_sock);
        return -1;
    }
    printf("cJSON_Parse success\n");

    cJSON *message=cJSON_GetObjectItem(root,"message");
    char *msg=cJSON_GetStringValue(message);
    printf("message:%s\n",msg);

    cJSON *status=cJSON_GetObjectItem(root,"status");
    int sta=status->valueint;
    printf("status:%d\n",sta);

    cJSON *date=cJSON_GetObjectItem(root,"date");
    char *dat=cJSON_GetStringValue(date);
    printf("data:%s\n",dat);

    cJSON *time=cJSON_GetObjectItem(root,"time");
    char *tim=cJSON_GetStringValue(time);
    printf("time:%s\n",tim);

    cJSON *cityInfo=cJSON_GetObjectItem(root,"cityInfo");
    cJSON *city=cJSON_GetObjectItem(cityInfo,"city");
    char *cit=cJSON_GetStringValue(city);
    printf("city:%s\n",cit);

    cJSON *citykey=cJSON_GetObjectItem(cityInfo,"citykey");
    char *citk=cJSON_GetStringValue(citykey);
    printf("citykey:%s\n",citk);

    cJSON *parent=cJSON_GetObjectItem(cityInfo,"parent");
    char *par=cJSON_GetStringValue(parent);
    printf("parent:%s\n",par);

    cJSON *updateTime=cJSON_GetObjectItem(cityInfo,"updateTime");
    char *upt=cJSON_GetStringValue(updateTime);
    printf("updateTime:%s\n",upt);

    //提取data对象
    cJSON *data=cJSON_GetObjectItem(root,"data");
    //提取shidu
    cJSON *shidu=cJSON_GetObjectItem(data,"shidu");
    char *shi=cJSON_GetStringValue(shidu);
    printf("shidu:%s\n",shi);

    //提取pm25
    cJSON *pm25=cJSON_GetObjectItem(data,"pm25");
    double pm=pm25->valuedouble;
    printf("pm25:%.1lf\n",pm);

    //提取pm10
    cJSON *pm10=cJSON_GetObjectItem(data,"pm10");
    double pmm=pm10->valuedouble;
    printf("pm10:%.1lf\n",pmm);

    //提取quality
    cJSON *quality=cJSON_GetObjectItem(data,"quality");
    char *qua=cJSON_GetStringValue(quality);
    printf("quality:%s\n",qua);

    //提取wendu
    cJSON *wendu=cJSON_GetObjectItem(data,"wendu");
    char *wen=cJSON_GetStringValue(wendu);
    printf("wendu:%s\n",wen);

    lv_label_set_text(ui_todaytemp,wen);
    lv_label_set_text(ui_todaytemp2,wen);

    //提取ganmao
    cJSON *ganmao=cJSON_GetObjectItem(data,"ganmao");
    char *gmao=cJSON_GetStringValue(ganmao);
    printf("ganmao:%s\n",gmao);

    printf("\n");

    cJSON *forecast_arry=cJSON_GetObjectItem(data,"forecast");
    for(int i=0;i<5;i++)
    {
        cJSON *forecast=cJSON_GetArrayItem(forecast_arry,i);

        //date
        cJSON *date=cJSON_GetObjectItem(forecast,"date");
        char *da=cJSON_GetStringValue(date);
        printf("date:%s\n",da);

        //high
        cJSON *high=cJSON_GetObjectItem(forecast,"high");
        char *hig=cJSON_GetStringValue(high);
        printf("high:%s\n",hig);

        //low
        cJSON *low=cJSON_GetObjectItem(forecast,"low");
        char *l=cJSON_GetStringValue(low);
        printf("low:%s\n",l);

        //ymd
        cJSON *ymd=cJSON_GetObjectItem(forecast,"ymd");
        char *y=cJSON_GetStringValue(ymd);
        printf("ymd:%s\n",y);

        //week
        cJSON *week=cJSON_GetObjectItem(forecast,"week");
        char *w=cJSON_GetStringValue(week);
        printf("week:%s\n",w);

        //sunrise
        cJSON *sunrise=cJSON_GetObjectItem(forecast,"sunrise");
        char *sunri=cJSON_GetStringValue(sunrise);
        printf("sunrise:%s\n",sunri);

        //sunset
        cJSON *sunset=cJSON_GetObjectItem(forecast,"sunset");
        char *suns=cJSON_GetStringValue(sunset);
        printf("sunset:%s\n",suns);

        //aqi
        cJSON *aqi=cJSON_GetObjectItem(forecast,"aqi");
        int aqi_num=aqi->valueint;
        printf("aqi:%d\n",aqi_num);

        //fx
        cJSON *fx=cJSON_GetObjectItem(forecast,"fx");
        char *fxs=cJSON_GetStringValue(fx);
        printf("fx:%s\n",fxs);

        //fl
        cJSON *fl=cJSON_GetObjectItem(forecast,"fl");
        char *fls=cJSON_GetStringValue(fl);
        printf("fl:%s\n",fls);

        //type
        cJSON *type=cJSON_GetObjectItem(forecast,"type");
        char *typ=cJSON_GetStringValue(type);
        printf("type:%s\n",typ);

        //notice
        cJSON *notice=cJSON_GetObjectItem(forecast,"notice");
        char *notic=cJSON_GetStringValue(notice);
        printf("notice:%s\n",notic);

        if(i==0)
        {
            if(strcmp(typ,"晴")==0)
            {
                lv_image_set_src(ui_todaywth, wether[6]);
                lv_image_set_src(ui_todaywth2, wether[6]);
            }
            else if(strcmp(typ,"多云")==0)
            {
                lv_image_set_src(ui_todaywth, wether[7]);
                lv_image_set_src(ui_todaywth2, wether[7]);
            }
            else if(strcmp(typ,"阴")==0)
            {
                lv_image_set_src(ui_todaywth, wether[8]);
                lv_image_set_src(ui_todaywth2, wether[8]);
            }
            else if(strcmp(typ,"小雨")==0)
            {
                lv_image_set_src(ui_todaywth, wether[9]);
                lv_image_set_src(ui_todaywth2, wether[9]);
            }
            else if(strcmp(typ,"中雨")==0)
            {
                lv_image_set_src(ui_todaywth, wether[10]);
                lv_image_set_src(ui_todaywth2, wether[10]);
            }
            else if(strcmp(typ,"大雨")==0)
            {
                lv_image_set_src(ui_todaywth, wether[11]);
                lv_image_set_src(ui_todaywth2, wether[11]);
            }
            choice_weather(ui_day1wth,typ);
            choice_week(ui_week1,w,ui_day1h,hig,ui_day1l,l);
        }
        else if(i==1)
        {
            choice_weather(ui_day2wth,typ);
            choice_week(ui_week2,w,ui_day2h,hig,ui_day2l,l);
        }
        else if(i==2)
        {
            choice_weather(ui_day3wth,typ);
            choice_week(ui_week3,w,ui_day3h,hig,ui_day3l,l);
        }
        else if(i==3)
        {
            choice_weather(ui_day4wth,typ);
            choice_week(ui_week4,w,ui_day4h,hig,ui_day4l,l);
        }
        else if(i==4)
        {
            choice_weather(ui_day5wth,typ);
            choice_week(ui_week5,w,ui_day5h,hig,ui_day5l,l);
        }
        printf("\n");
    }
}