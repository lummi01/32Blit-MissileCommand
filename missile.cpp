// Missile Command
// 2024 M. Gerloff

#include <list>
#include "missile.hpp"
#include "assets.hpp"

using namespace blit;

Font font(font3x5);

struct GAME
{
    int state;
    short level;
};

struct PLAYER
{
    int score;
    Vec2 pos;
    short city[6]{10,10,10,10,10,10};
    short shot[3];
    short mag[3];
    bool load[3];
};

struct SHOT
{
    bool is;
    Vec2 pos;
    Vec2 vel;
    Vec2 target;
};

struct EXPLOSION
{
	Vec2 pos;
	int alpha;
    float radius;
    float rdx;
};

struct MISSILE
{
    short type;
    Vec2 start;
    Vec2 pos;
    Vec2 vel;
    short target;
};

static std::list<EXPLOSION> particles;

GAME game;
PLAYER p;
SHOT shot[4];
MISSILE missile[9];

Timer base_timer;
Timer missile_timer;


void NewExplosion(Vec2 pos)
{
    EXPLOSION e;
    e.pos = pos;
    e.alpha = rand() %256;
    e.radius = .25f;
    e.rdx = .25f;
    particles.push_back(e);
}

void UpdateExplosion()
{
    for(auto e = particles.begin(); e != particles.end();) 
	{
        if(e->radius < .25f) 
		{
            e = particles.erase(e);
            continue;
        }
        e->alpha = rand() %256;;
        e->radius += e->rdx;
        if (e->radius > 13)
            e->rdx = -.25f;
        ++e;
    }
}

void NewMissile(short type)
{
    short anz[7]{1,2,2,2,2,3,3};
    short space[7]{0,1,2,3,4,1,2};
    
    int target = rand() %(9 - ((anz[type - 1] - 1) * space[type - 1]));
    int x0 = 20 + rand() %(120 - ((anz[type - 1] - 1) * space[type -1] * 20));
    int x1 = target * 20;
    int y0 = 0; 

    for (short t=0; t<anz[type - 1]; t++)
    {
        for (short i=0; i<9; i++)
        {
            if (missile[i].type == 0)
            {
                missile[i].type = type;
                missile[i].start = Vec2(x0 + (t * (space[type -1] * 20)), y0);
                missile[i].pos = missile[i].start;
                missile[i].target = target + (t * (space[type -1]));
                float dx = x1 + (t * (space[type - 1] * 20)) - missile[i].start.x;
                float dy = 110 - missile[i].start.y;
                float s = sqrt((dx * dx) + (dy * dy)) * 8;
                missile[i].vel = Vec2(dx / s, dy / s);
                break;
            }
        }
    }
}

void UpdateMissile()
{
    bool missiles = false;

    for (short i=0; i<9; i++)
    {
        if (missile[i].type > 0)
        {
            missiles = true;

            missile[i].pos += missile[i].vel;
            if (missile[i].pos.y > 110)
            {
                if (missile[i].target == 0 || missile[i].target == 4 || missile[i].target == 8)
                {
                    short t = missile[i].target / 4;
                    p.shot[t] = 0;
                    if (p.mag[t] > 0)
                    {
                        p.load[t] = true;
                        p.mag[t]--;
                    }
                }
                else if (missile[i].target < 4)
                {
                    p.city[missile[i].target - 1] = 12;
                }
                else
                {
                    p.city[missile[i].target - 2] = 12;
                }

                missile[i].type = 0;
                NewExplosion(missile[i].pos);
            }
            else
            {
                for(auto &e : particles) 
            	{
                    float dx = e.pos.x - missile[i].pos.x;
                    float dy = e.pos.y - missile[i].pos.y;
                    float d = sqrt((dx * dx) + (dy * dy));
                    if (d < e.radius)
                    {
                        missile[i].type = 0;
                        NewExplosion(missile[i].pos);
                        break;
                    }
                }
            }
        }
    }
    
    if (missiles == false)
    {
        NewMissile(1 + rand() %7);
        missile_timer.start();
    }
}

void Missile(Timer &t)
{
    NewMissile(1 + rand() %7);
    missile_timer.init(Missile, (1 + rand() %10) * 1000, 1);
    missile_timer.start();
}

void NewShot(short i)
{
    if (p.shot[i] > 0 && p.load[i] == false)
    {
        for (short t=0; t<4; t++)
        {
            if (shot[t].is == false)
            {
                p.shot[i]--;
                if (p.shot[i] == 0 && p.mag[i] > 0)
                {
                    p.load[i] = true;
                    p.mag[i]--;
                }

                float dx = (i * 80) - p.pos.x;
                float dy = 106 - p.pos.y;
                float s = sqrt((dx * dx) + (dy * dy));

                shot[t].is = true;
                shot[t].pos = Vec2(i * 80, 106);
                shot[t].vel = Vec2(dx / s, dy / s);
                shot[t].target = Vec2(p.pos.x, p.pos.y);
                break;
            }
        }
    }
}

void UpdateShot()
{
    for (short s=0; s<4; s++)
    {
        if (shot[s].is)
        {
            shot[s].pos -= shot[s].vel;
            if (shot[s].pos.y <= shot[s].target.y)
            {
                shot[s].is = false;
                NewExplosion(shot[s].target);
            }
        }
    }
}

void UpdateBase(Timer &t)
{
    for (short i=0; i<3; i++)
    {
        if (p.load[i])
        {
            short s[3]{6,9,6};
            p.shot[i]++;
            if (p.shot[i] == s[i])
                p.load[i] = false;
        }
    }
}

void UpdateControl()
{
    p.pos += Vec2(joystick.x * 2,joystick.y * 2);
    
    if (buttons & Button::DPAD_LEFT)
        p.pos.x -= 2;
    else if (buttons & Button::DPAD_RIGHT)
        p.pos.x += 2;
    if (buttons & Button::DPAD_UP)
        p.pos.y -= 2;
    else if (buttons & Button::DPAD_DOWN || joystick.y > 0)
        p.pos.y += 2;

    if (p.pos.x < 0)
        p.pos.x = 0;
    else if (p.pos.x > 159)
        p.pos.x = 159;
    if (p.pos.y < 0)
        p.pos.y = 0;
    else if (p.pos.y > 105)
        p.pos.y = 105;

    if (buttons.pressed & Button::Y)
        NewShot(0);
    if (buttons.pressed & Button::X)
        NewShot(1);
    if (buttons.pressed & Button::A)
        NewShot(2);
}

// init()
void init() 
{
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(sprites);

    base_timer.init(UpdateBase, 100, -1);
    base_timer.start();
    missile_timer.init(Missile, 10000, 1);
    missile_timer.start();

    p.pos = Vec2(79,59);

    for (short i=0; i<3; i++)
    {
        p.load[i] = true;
        p.mag[i] = 3;
    }

    NewMissile(1 + rand() %7);
}

// render()
void render(uint32_t time) 
{
    screen.clear();

    screen.mask = nullptr;

    //ground
    screen.sprite(Rect(0, 0, 10, 2), Point(0, 106));
    screen.sprite(Rect(0, 0, 10, 2), Point(80, 106));

    for (short i=0; i<3; i++)
    {
        //citys
        screen.sprite(Rect(p.city[i], 0, 2, 1), Point(12 + (i * 20), 108));
        screen.sprite(Rect(p.city[i + 3], 0, 2, 1), Point(92 + (i * 20), 108));
        
        //rockets in base
        Vec2 rpos[3][9]{{Vec2(-1,116),Vec2(1,116),Vec2(-1,113),Vec2(3,116),Vec2(1,113),Vec2(-1,110),Vec2(0,0),Vec2(0,0),Vec2(0,0)},
                       {Vec2(78,116),Vec2(80,116),Vec2(76,116),Vec2(78,113),Vec2(82,116),Vec2(74,116),Vec2(80,113),Vec2(76,113),Vec2(78,110)},
                       {Vec2(158,116),Vec2(156,116),Vec2(158,113),Vec2(154,116),Vec2(156,113),Vec2(158,110),Vec2(0,0),Vec2(0,0),Vec2(0,0)}};
        for (short r=0; r<p.shot[i]; r++)
            screen.sprite(14, Point(rpos[i][r].x, rpos[i][r].y));
    }

    screen.sprite(15, Point(p.pos.x - 2, p.pos.y - 2));

    screen.pen = Pen(0, 0, 255);
    for (short i=0; i<9; i++)
    {
        if (missile[i].type > 0)
        {
            for (int t=missile[i].start.y; t<missile[i].pos.y; t++)
                screen.pixel(Point(missile[i].start.x + (missile[i].vel.x * (t / missile[i].vel.y)), t));
        }
    }         
    screen.pen = Pen(255, 0, 0);
    for (short i=0; i<9; i++)
    {
        if (missile[i].type > 0)
            screen.pixel(Point(missile[i].pos.x, missile[i].pos.y));
    }         

    //explosion
    screen.pen = Pen(255, 255, 255);
    for(auto &e : particles)
    {
    	screen.alpha = e.alpha;
        screen.circle(Point(e.pos.x, e.pos.y),e.radius);
    }

    screen.alpha = 255;

    for (short s=0; s<4; s++) //rockets
    {
        if (shot[s].is)
            screen.pixel(Point(shot[s].pos.x, shot[s].pos.y));
    }

    std::string score_txt ("000000");
    std::string score (std::to_string(p.score));
    score_txt.erase(0, score.size());
    screen.text(score_txt + score, font, Point(80, 1), true, TextAlign::top_center);        
    screen.pen = Pen(0, 0, 0);
}

// update()
void update(uint32_t time) 
{
    UpdateControl();
    UpdateShot();
    UpdateExplosion();
    UpdateMissile();
}
