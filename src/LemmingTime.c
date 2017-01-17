#include <pebble.h>

#define ANIM_SPEED 45
#define NUMBER_OF_LEMMINGS 14
#define FLOW_RATE 6
static Window *window;
GBitmap *day, *hour[2], *minute[2], *month[2], *mday[2], *year[2], *colon, *slash[2], *AMPM, *blocker, *gohome, *death, *home_dragon, *spinner, *trapdoor, *lemming_bitmap[NUMBER_OF_LEMMINGS];
BitmapLayer *day_layer, *colon_layer, *slash_layer[2], *hour_layer[2], *minute_layer[2], *month_layer[2], *mday_layer[2], *year_layer[2], *AMPM_layer, *blocker_layer, *gohome_layer, *death_layer, *home_layer, *spinner_layer, *trapdoor_layer, *lemming_layer[NUMBER_OF_LEMMINGS];
GBitmapSequence *blocker_anim, *gohome_anim, *death_anim, *home_anim, *spinner_anim, *trapdoor_anim, *lemming_anim[NUMBER_OF_LEMMINGS][4];
AppTimer *timer_handle;
uint8_t colon_toggle = 1, time_position = 33, new_minute, rate = 2, random_flow = FLOW_RATE, l_count = 0;
uint8_t lemming_type[NUMBER_OF_LEMMINGS], lemming_x[NUMBER_OF_LEMMINGS], lemming_y[NUMBER_OF_LEMMINGS];
int lemming_d[NUMBER_OF_LEMMINGS];

void lemming_reset() {
    
    for (int i=0;i<NUMBER_OF_LEMMINGS;i++) {

        lemming_type[i] = 2;
#ifdef PBL_ROUND
        lemming_x[i] = 85;
        lemming_y[i] = 23;
#else
        lemming_x[i] = 67;
        lemming_y[i] = 19;
#endif
        lemming_d[i] = 1;
        //starts hidden
        layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
        rate = 2;
        random_flow = rand() % FLOW_RATE + 1;
        l_count = 0;

    }
    layer_set_hidden(bitmap_layer_get_layer(blocker_layer), true);
}

void lemming_init() {
    
    for (int i=0;i<NUMBER_OF_LEMMINGS;i++) {
        
        lemming_type[i] = 2;
#ifdef PBL_ROUND
        lemming_x[i] = 85;
        lemming_y[i] = 23;
#else
        lemming_x[i] = 67;
        lemming_y[i] = 19;
#endif
        lemming_d[i] = 1;
        //starts hidden
        lemming_anim[i][0] = gbitmap_sequence_create_with_resource(23); //walk right
        lemming_anim[i][1] = gbitmap_sequence_create_with_resource(24); //walk left
        lemming_anim[i][2] = gbitmap_sequence_create_with_resource(25); //fall right
        lemming_anim[i][3] = gbitmap_sequence_create_with_resource(26); //fall left
        lemming_bitmap[i] = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(lemming_anim[i][0]), GBitmapFormat8Bit);
        lemming_layer[i] = bitmap_layer_create(GRect(lemming_x[i], lemming_y[i], 10, 10));
        bitmap_layer_set_compositing_mode(lemming_layer[i], GCompOpSet);
        layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
        layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(lemming_layer[i]));

    }
}


void anim_update(GBitmapSequence *anim, BitmapLayer *anim_layer, GBitmap *anim_bitmap, uint8_t light) {
    
    if ((anim) && (anim_bitmap)) {
        if(gbitmap_sequence_update_bitmap_next_frame(anim, anim_bitmap, NULL)) {
            bitmap_layer_set_bitmap(anim_layer, anim_bitmap);
        } else {
            if (light == 0) {
                layer_set_hidden(bitmap_layer_get_layer(anim_layer), true);
            }
        }
    }
}

void handle_timer() {
    
    anim_update(blocker_anim, blocker_layer, blocker, 1);
    anim_update(gohome_anim, gohome_layer, gohome, 0);
    anim_update(death_anim, death_layer, death, 0);
    anim_update(home_anim, home_layer, home_dragon, 1);
    anim_update(spinner_anim, spinner_layer, spinner, 1);
    anim_update(trapdoor_anim, trapdoor_layer, trapdoor, 1);
    
    for (int i=0;i<NUMBER_OF_LEMMINGS;i++) {
        
        anim_update(lemming_anim[i][lemming_type[i]], lemming_layer[i], lemming_bitmap[lemming_type[i]], 1);
    }
#ifdef PBL_ROUND
    for (int i=0;i<NUMBER_OF_LEMMINGS;i++) {
        
        if (lemming_type[i] < 2) { //walkers

            lemming_x[i] = lemming_x[i] + lemming_d[i];
            if ((lemming_x[i] > 164) || ((lemming_x[i] == 80) && (lemming_y[i] == 139) && (layer_get_hidden(bitmap_layer_get_layer(blocker_layer)) == false))) {
                lemming_d[i] = -1;
                lemming_type[i] = 1;
            }
            if (lemming_x[i] == 8) {
                lemming_d[i] = 1;
                lemming_type[i] = 0;
            }
            
            if ((lemming_x[i] == 30) && (lemming_y[i] == 63)) { //First waypoint
                
                lemming_type[i] = 3; //left faller
            }
            if ((lemming_x[i] == 68) && (lemming_y[i] == 115)) { //Third waypoint
                
                lemming_type[i] = 2; //right faller
            }
            if ((lemming_x[i] == 130) && (lemming_y[i] == 139)) { //Lemming reached Home
                
                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(gohome_layer), false);
                gbitmap_sequence_restart(gohome_anim);
            }
            if ((lemming_x[i] == 84) && (lemming_y[i] == 139) && (rand() % NUMBER_OF_LEMMINGS == 0)) { //Random Blocker
                
                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(blocker_layer), false);
            }
            if ((lemming_x[i] < 44) && (lemming_y[i] == 139)) { //Death by spinner

                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(death_layer), false);
                gbitmap_sequence_restart(death_anim);
                
                
                
                
                
            }

        }
        
        if ((lemming_type[i] > 1) && (lemming_type[i] < 4) && (layer_get_hidden(bitmap_layer_get_layer(lemming_layer[i])) == false)) { //fallers
            
            lemming_y[i] = lemming_y[i] + 4;
            if (lemming_y[i] == 63) {
                if (lemming_d[i] == 1) {
                    
                    lemming_type[i] = 0;
                } else  {
                    
                    lemming_type[i] = 1;
                }
            }
            if (lemming_y[i] == 115) { //Second waypoint
                
                lemming_type[i] = 1; //left walker
                }
            if (lemming_y[i] == 139) { //Fourth waypoint
                
                lemming_type[i] = 0; //right walker
                }
            
            
        }
        
        layer_set_frame(bitmap_layer_get_layer(lemming_layer[i]), GRect(lemming_x[i], lemming_y[i], 10, 10));

    }
#else 
    for (int i=0;i<NUMBER_OF_LEMMINGS;i++) {
        
        if (lemming_type[i] < 2) { //walkers

            lemming_x[i] = lemming_x[i] + lemming_d[i];
            if ((lemming_x[i] > 134) || ((lemming_x[i] == 62) && (lemming_y[i] == 139) && (layer_get_hidden(bitmap_layer_get_layer(blocker_layer)) == false))) {
                lemming_d[i] = -1;
                lemming_type[i] = 1;
            }
            if (lemming_x[i] == 0) {
                lemming_d[i] = 1;
                lemming_type[i] = 0;
            }
            
            if ((lemming_x[i] == 12) && (lemming_y[i] == 63)) { //First waypoint
                
                lemming_type[i] = 3; //left faller
            }
            if ((lemming_x[i] == 50) && (lemming_y[i] == 115)) { //Third waypoint
                
                lemming_type[i] = 2; //right faller
            }
            if ((lemming_x[i] == 112) && (lemming_y[i] == 139)) { //Lemming reached Home
                
                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(gohome_layer), false);
                gbitmap_sequence_restart(gohome_anim);
            }
            if ((lemming_x[i] == 66) && (lemming_y[i] == 139) && (rand() % NUMBER_OF_LEMMINGS == 0)) { //Random Blocker
                
                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(blocker_layer), false);
            }
            if ((lemming_x[i] < 26) && (lemming_y[i] == 139)) { //Death by spinner

                lemming_type[i] = 2;
                layer_set_hidden(bitmap_layer_get_layer(lemming_layer[i]), true);
                layer_set_hidden(bitmap_layer_get_layer(death_layer), false);
                gbitmap_sequence_restart(death_anim);
                
                
                
                
                
            }

        }
        
        if ((lemming_type[i] > 1) && (lemming_type[i] < 4) && (layer_get_hidden(bitmap_layer_get_layer(lemming_layer[i])) == false)) { //fallers
            
            lemming_y[i] = lemming_y[i] + 4;
            if (lemming_y[i] == 63) {
                if (lemming_d[i] == 1) {
                    
                    lemming_type[i] = 0;
                } else  {
                    
                    lemming_type[i] = 1;
                }
            }
            if (lemming_y[i] == 115) { //Second waypoint
                
                lemming_type[i] = 1; //left walker
                }
            if (lemming_y[i] == 139) { //Fourth waypoint
                
                lemming_type[i] = 0; //right walker
                }
            
            
        }
        
        layer_set_frame(bitmap_layer_get_layer(lemming_layer[i]), GRect(lemming_x[i], lemming_y[i], 10, 10));

    }
#endif
    layer_mark_dirty(window_get_root_layer(window));
    timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
    
    if (colon_toggle == 1) {
        
        layer_set_hidden(bitmap_layer_get_layer(colon_layer), false);
        colon_toggle = 0;
    } else {
        layer_set_hidden(bitmap_layer_get_layer(colon_layer), true);
        colon_toggle = 1;
    }
    
    if (tick_time->tm_min != new_minute) {
        new_minute = tick_time->tm_min;
        gbitmap_sequence_restart(trapdoor_anim);
        lemming_reset();
    }
    

    if ((l_count < NUMBER_OF_LEMMINGS) && (rate == 0)) {
        
        layer_set_hidden(bitmap_layer_get_layer(lemming_layer[l_count]), false);
        l_count++;
        rate = random_flow;
        }
    if (l_count < NUMBER_OF_LEMMINGS) rate = rate - 1;
    
    gbitmap_destroy(day);
    day = gbitmap_create_with_resource((tick_time->tm_wday + 12));
    bitmap_layer_set_bitmap(day_layer, day);
    
    gbitmap_destroy(minute[0]);
    gbitmap_destroy(minute[1]);
    gbitmap_destroy(hour[0]);
    gbitmap_destroy(hour[1]);
    gbitmap_destroy(month[0]);
    gbitmap_destroy(month[1]);
    gbitmap_destroy(mday[0]);
    gbitmap_destroy(mday[1]);
    gbitmap_destroy(year[0]);
    gbitmap_destroy(year[1]);
    
    month[0] = gbitmap_create_with_resource(((tick_time->tm_mon + 1) / 10) + 2);
    month[1] = gbitmap_create_with_resource(((tick_time->tm_mon + 1) % 10) + 2);
    mday[0] = gbitmap_create_with_resource((tick_time->tm_mday / 10) + 2);
    mday[1] = gbitmap_create_with_resource((tick_time->tm_mday % 10) + 2);
    year[0] = gbitmap_create_with_resource(((tick_time->tm_year - 100) / 10) + 2);
    year[1] = gbitmap_create_with_resource(((tick_time->tm_year - 100) % 10) + 2);

    if (clock_is_24h_style()) {
    
        hour[0] = gbitmap_create_with_resource((tick_time->tm_hour / 10) + 2);
        hour[1] = gbitmap_create_with_resource((tick_time->tm_hour % 10) + 2);

    } else {
        if ((tick_time->tm_hour == 12) || (tick_time->tm_hour == 0)) {
            hour[0] = gbitmap_create_with_resource(3);
            hour[1] = gbitmap_create_with_resource(4);
        } else {
            hour[0] = gbitmap_create_with_resource(((tick_time->tm_hour % 12) / 10) + 2);
            hour[1] = gbitmap_create_with_resource(((tick_time->tm_hour % 12) % 10) + 2);
        }
        if (((tick_time->tm_hour > 9) && (tick_time->tm_hour < 13)) || ((tick_time->tm_hour > 21) && (tick_time->tm_hour < 24)) || (tick_time->tm_hour == 0)) {
            layer_set_hidden(bitmap_layer_get_layer(hour_layer[0]), false);
            time_position = 33;
        } else {
            layer_set_hidden(bitmap_layer_get_layer(hour_layer[0]), true);
            time_position = 25;
        }
        gbitmap_destroy(AMPM);
        if (tick_time->tm_hour >= 12) {
            AMPM = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PM);
        } else {
            AMPM = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AM);
        }
        bitmap_layer_set_bitmap(AMPM_layer, AMPM);
    }
    minute[0] = gbitmap_create_with_resource((tick_time->tm_min / 10) + 2);
    minute[1] = gbitmap_create_with_resource((tick_time->tm_min % 10) + 2);
    
    bitmap_layer_set_bitmap(hour_layer[0], hour[0]);
    bitmap_layer_set_bitmap(hour_layer[1], hour[1]);
    bitmap_layer_set_bitmap(minute_layer[0], minute[0]);
    bitmap_layer_set_bitmap(minute_layer[1], minute[1]);
    bitmap_layer_set_bitmap(month_layer[0], month[0]);
    bitmap_layer_set_bitmap(month_layer[1], month[1]);
    bitmap_layer_set_bitmap(mday_layer[0], mday[0]);
    bitmap_layer_set_bitmap(mday_layer[1], mday[1]);
    bitmap_layer_set_bitmap(year_layer[0], year[0]);
    bitmap_layer_set_bitmap(year_layer[1], year[1]);
#ifdef PBL_ROUND   
    layer_set_frame(bitmap_layer_get_layer(hour_layer[0]),GRect(time_position+18, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(hour_layer[1]),GRect(time_position+35, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(minute_layer[0]),GRect(time_position+61, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(minute_layer[1]),GRect(time_position+78, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(colon_layer),GRect(time_position+52, 75, 9, 14));
#else       
    layer_set_frame(bitmap_layer_get_layer(hour_layer[0]),GRect(time_position, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(hour_layer[1]),GRect(time_position+17, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(minute_layer[0]),GRect(time_position+43, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(minute_layer[1]),GRect(time_position+60, 75, 17, 15));
    layer_set_frame(bitmap_layer_get_layer(colon_layer),GRect(time_position+34, 75, 9, 14));
#endif    
}

void handle_init(void) {
    
    window = window_create();
    window_stack_push(window, true /* Animated */);
    Layer *window_layer = window_get_root_layer(window);
    window_set_background_color(window, GColorBlack);
    timer_handle = app_timer_register(ANIM_SPEED, handle_timer, NULL);    
    day = gbitmap_create_with_resource(12);
    colon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLON);
    hour[0] = gbitmap_create_with_resource(2);
    hour[1] = gbitmap_create_with_resource(2);
    minute[0] = gbitmap_create_with_resource(2);
    minute[1] = gbitmap_create_with_resource(2);
    month[0] = gbitmap_create_with_resource(2);
    month[1] = gbitmap_create_with_resource(2);
    mday[0] = gbitmap_create_with_resource(2);
    mday[1] = gbitmap_create_with_resource(2);
    year[0] = gbitmap_create_with_resource(2);
    year[1] = gbitmap_create_with_resource(2);
    slash[0] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLASH);
    slash[1] = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SLASH);
 #ifdef PBL_ROUND   
    day_layer = bitmap_layer_create(GRect(66, 4, 48, 15));
    colon_layer = bitmap_layer_create(GRect(time_position+52, 75, 9, 14));
    hour_layer[0] = bitmap_layer_create(GRect(time_position+18, 75, 17, 15));
    hour_layer[1] = bitmap_layer_create(GRect(time_position+35, 75, 17, 15));
    minute_layer[0] = bitmap_layer_create(GRect(time_position+61, 75, 17, 15));
    minute_layer[1] = bitmap_layer_create(GRect(time_position+78, 75, 17, 15));
    month_layer[0] = bitmap_layer_create(GRect(22, 152, 17, 15));
    month_layer[1] = bitmap_layer_create(GRect(39, 152, 17, 15));
    mday_layer[0] = bitmap_layer_create(GRect(73, 152, 17, 15));
    mday_layer[1] = bitmap_layer_create(GRect(90, 152, 17, 15));
    year_layer[0] = bitmap_layer_create(GRect(124, 152, 17, 15));
    year_layer[1] = bitmap_layer_create(GRect(141, 152, 17, 15));
    slash_layer[0] = bitmap_layer_create(GRect(56, 152, 17, 16));
    slash_layer[1] = bitmap_layer_create(GRect(107, 152, 17, 16));
 
    if (!clock_is_24h_style()) {
        AMPM = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AM);
        AMPM_layer = bitmap_layer_create(GRect(73, 92, 34, 16));
        bitmap_layer_set_compositing_mode(AMPM_layer, GCompOpSet);
        bitmap_layer_set_bitmap(AMPM_layer, AMPM);
        layer_add_child(window_layer, bitmap_layer_get_layer(AMPM_layer));
    }
#else
    day_layer = bitmap_layer_create(GRect(48, 0, 48, 15));
    colon_layer = bitmap_layer_create(GRect(time_position+34, 75, 9, 14));
    hour_layer[0] = bitmap_layer_create(GRect(time_position, 75, 17, 15));
    hour_layer[1] = bitmap_layer_create(GRect(time_position+17, 75, 17, 15));
    minute_layer[0] = bitmap_layer_create(GRect(time_position+43, 75, 17, 15));
    minute_layer[1] = bitmap_layer_create(GRect(time_position+60, 75, 17, 15));
    month_layer[0] = bitmap_layer_create(GRect(4, 152, 17, 15));
    month_layer[1] = bitmap_layer_create(GRect(21, 152, 17, 15));
    mday_layer[0] = bitmap_layer_create(GRect(55, 152, 17, 15));
    mday_layer[1] = bitmap_layer_create(GRect(72, 152, 17, 15));
    year_layer[0] = bitmap_layer_create(GRect(106, 152, 17, 15));
    year_layer[1] = bitmap_layer_create(GRect(123, 152, 17, 15));
    slash_layer[0] = bitmap_layer_create(GRect(38, 152, 17, 16));
    slash_layer[1] = bitmap_layer_create(GRect(89, 152, 17, 16));
	   
    if (!clock_is_24h_style()) {
        AMPM = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_AM);
        AMPM_layer = bitmap_layer_create(GRect(55, 92, 34, 16));
        bitmap_layer_set_compositing_mode(AMPM_layer, GCompOpSet);
        bitmap_layer_set_bitmap(AMPM_layer, AMPM);
        layer_add_child(window_layer, bitmap_layer_get_layer(AMPM_layer));
    }
 #endif
	   
    bitmap_layer_set_compositing_mode(day_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(hour_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(hour_layer[1], GCompOpSet);
    bitmap_layer_set_compositing_mode(minute_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(minute_layer[1], GCompOpSet);
    bitmap_layer_set_compositing_mode(colon_layer, GCompOpSet);
    bitmap_layer_set_compositing_mode(month_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(month_layer[1], GCompOpSet);
    bitmap_layer_set_compositing_mode(mday_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(mday_layer[1], GCompOpSet);
    bitmap_layer_set_compositing_mode(year_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(year_layer[1], GCompOpSet);
    bitmap_layer_set_compositing_mode(slash_layer[0], GCompOpSet);
    bitmap_layer_set_compositing_mode(slash_layer[1], GCompOpSet);

    bitmap_layer_set_bitmap(day_layer, day);
    bitmap_layer_set_bitmap(hour_layer[0], hour[0]);
    bitmap_layer_set_bitmap(hour_layer[1], hour[1]);
    bitmap_layer_set_bitmap(minute_layer[0], minute[0]);
    bitmap_layer_set_bitmap(minute_layer[1], minute[1]);
    bitmap_layer_set_bitmap(colon_layer, colon);
    bitmap_layer_set_bitmap(month_layer[0], month[0]);
    bitmap_layer_set_bitmap(month_layer[1], month[1]);
    bitmap_layer_set_bitmap(mday_layer[0], mday[0]);
    bitmap_layer_set_bitmap(mday_layer[1], mday[1]);
    bitmap_layer_set_bitmap(year_layer[0], year[0]);
    bitmap_layer_set_bitmap(year_layer[1], year[1]);
    bitmap_layer_set_bitmap(slash_layer[0], slash[0]);
    bitmap_layer_set_bitmap(slash_layer[1], slash[1]);
    
    layer_add_child(window_layer, bitmap_layer_get_layer(day_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(colon_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(hour_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(hour_layer[1]));
    layer_add_child(window_layer, bitmap_layer_get_layer(minute_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(minute_layer[1]));
    layer_add_child(window_layer, bitmap_layer_get_layer(month_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(month_layer[1]));
    layer_add_child(window_layer, bitmap_layer_get_layer(mday_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(mday_layer[1]));
    layer_add_child(window_layer, bitmap_layer_get_layer(year_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(year_layer[1]));
    layer_add_child(window_layer, bitmap_layer_get_layer(slash_layer[0]));
    layer_add_child(window_layer, bitmap_layer_get_layer(slash_layer[1]));
#ifdef PBL_ROUND    
    blocker_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_BLOCKER);
    blocker = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(blocker_anim), GBitmapFormat8Bit);
    blocker_layer = bitmap_layer_create(GRect(84, 139, 12, 10));
    bitmap_layer_set_compositing_mode(blocker_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(blocker_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(blocker_layer));
    
    home_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_HOME_DRAGON);
    home_dragon = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(home_anim), GBitmapFormat8Bit);
    home_layer = bitmap_layer_create(GRect(116, 98, 44, 52));
    bitmap_layer_set_compositing_mode(home_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(home_layer));
    
    gohome_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_GO_HOME);
    gohome = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(gohome_anim), GBitmapFormat8Bit);
    gohome_layer = bitmap_layer_create(GRect(130, 138, 10, 13));
    bitmap_layer_set_compositing_mode(gohome_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(gohome_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(gohome_layer));
    
    death_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_DEATH);
    death = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(death_anim), GBitmapFormat8Bit);
    death_layer = bitmap_layer_create(GRect(41, 138, 12, 14));
    bitmap_layer_set_compositing_mode(death_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(death_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(death_layer));

    spinner_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_SPINNER);
    spinner = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(spinner_anim), GBitmapFormat8Bit);
    spinner_layer = bitmap_layer_create(GRect(19, 131, 30, 19));
    bitmap_layer_set_compositing_mode(spinner_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(spinner_layer));
    
    trapdoor_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_TRAPDOOR_1);
    trapdoor = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(trapdoor_anim), GBitmapFormat8Bit);
    trapdoor_layer = bitmap_layer_create(GRect(69, 20, 41, 24));
    bitmap_layer_set_compositing_mode(trapdoor_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(trapdoor_layer));
#else       
    blocker_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_BLOCKER);
    blocker = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(blocker_anim), GBitmapFormat8Bit);
    blocker_layer = bitmap_layer_create(GRect(66, 139, 12, 10));
    bitmap_layer_set_compositing_mode(blocker_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(blocker_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(blocker_layer));
    
    home_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_HOME_DRAGON);
    home_dragon = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(home_anim), GBitmapFormat8Bit);
    home_layer = bitmap_layer_create(GRect(98, 98, 44, 52));
    bitmap_layer_set_compositing_mode(home_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(home_layer));
    
    gohome_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_GO_HOME);
    gohome = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(gohome_anim), GBitmapFormat8Bit);
    gohome_layer = bitmap_layer_create(GRect(112, 138, 10, 13));
    bitmap_layer_set_compositing_mode(gohome_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(gohome_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(gohome_layer));
    
    death_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_DEATH);
    death = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(death_anim), GBitmapFormat8Bit);
    death_layer = bitmap_layer_create(GRect(23, 138, 12, 14));
    bitmap_layer_set_compositing_mode(death_layer, GCompOpSet);
    layer_set_hidden(bitmap_layer_get_layer(death_layer), true);
    layer_add_child(window_layer, bitmap_layer_get_layer(death_layer));

    spinner_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_SPINNER);
    spinner = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(spinner_anim), GBitmapFormat8Bit);
    spinner_layer = bitmap_layer_create(GRect(1, 131, 30, 19));
    bitmap_layer_set_compositing_mode(spinner_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(spinner_layer));
    
    trapdoor_anim = gbitmap_sequence_create_with_resource(RESOURCE_ID_TRAPDOOR_1);
    trapdoor = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(trapdoor_anim), GBitmapFormat8Bit);
    trapdoor_layer = bitmap_layer_create(GRect(51, 16, 41, 24));
    bitmap_layer_set_compositing_mode(trapdoor_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(trapdoor_layer));
 #endif   
    srand(time(NULL));
    random_flow = rand() % FLOW_RATE + 1;
    lemming_init();
    
    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    new_minute = current_time->tm_min;
    handle_second_tick(current_time, SECOND_UNIT);
    tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

}

void handle_deinit(void) {
    
    tick_timer_service_unsubscribe();
    gbitmap_destroy(day);
    gbitmap_destroy(hour[0]);
    gbitmap_destroy(hour[1]);
    gbitmap_destroy(minute[0]);
    gbitmap_destroy(minute[1]);
    gbitmap_destroy(colon);
    gbitmap_destroy(slash[0]);
    gbitmap_destroy(slash[1]);
    gbitmap_destroy(month[0]);
    gbitmap_destroy(month[1]);
    gbitmap_destroy(mday[0]);
    gbitmap_destroy(mday[1]);
    gbitmap_destroy(year[0]);
    gbitmap_destroy(year[1]);
    
    if (!clock_is_24h_style()) {
        gbitmap_destroy(AMPM);
        bitmap_layer_destroy(AMPM_layer);
    }
    bitmap_layer_destroy(day_layer);
    bitmap_layer_destroy(hour_layer[0]);
    bitmap_layer_destroy(hour_layer[1]);
    bitmap_layer_destroy(minute_layer[0]);
    bitmap_layer_destroy(minute_layer[1]);
    bitmap_layer_destroy(colon_layer);
    bitmap_layer_destroy(slash_layer[0]);
    bitmap_layer_destroy(slash_layer[1]);
    bitmap_layer_destroy(month_layer[0]);
    bitmap_layer_destroy(month_layer[1]);
    bitmap_layer_destroy(mday_layer[0]);
    bitmap_layer_destroy(mday_layer[1]);
    bitmap_layer_destroy(year_layer[0]);
    bitmap_layer_destroy(year_layer[1]);
    
    window_destroy(window);
    
}

int main(void) {
    
    handle_init();
    app_event_loop();
    handle_deinit();
    
}
