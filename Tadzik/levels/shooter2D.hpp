#ifndef SHOOTER2D_HPP
#define SHOOTER2D_HPP

#include "../include/Scene.hpp"
#include "../include/AnimatedSprite.hpp"
#include "../include/Collision.hpp"
#include "../include/Utils.hpp"
#include "../include/Common.hpp"

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <fstream>

class SHOOTER2D: public Scene{
public:
    SHOOTER2D(std::string _name, SceneManager* mgr, sf::RenderWindow* w)
        :Scene(_name, mgr, w)
    {}
    friend class Powerup;

    class HUD: public sf::RenderTexture {
    public:
        HUD() {
            tAmmo.setPosition(1200, 690);
            tAllAmmo.setPosition(1250, 690);
            tScore.setPosition(1250, 0);
            healthBar.setPosition(10, 5);
            healthFrame.setPosition(10, 5);
            activeWeapon.setPosition(1150, 600);
            reloading.setSize(sf::Vector2f(30, 10));
            reloading.setFillColor(sf::Color::Green);
        }
        sf::Sprite healthBar;
        sf::Sprite healthFrame;
        sf::Sprite frame;
        sf::Sprite activeWeapon;
        sf::RectangleShape reloading;
        sf::Text tScore;
        sf::Text tAmmo;
        sf::Text tAllAmmo;
        void update() {
            draw(frame);
            draw(healthFrame);
            draw(healthBar);
            draw(tScore);
            draw(tAmmo);
            draw(tAllAmmo);
            draw(activeWeapon);
            display();
        }
        void setFont(sf::Font* f, sf::Color c) {
            tScore.setFont(*f);
            tAmmo.setFont(*f);
            tAllAmmo.setFont(*f);
            tScore.setColor(c);
            tAmmo.setColor(c);
            tAllAmmo.setColor(c);
        }
    };

    class Wave {
    public:

        Wave(float t, int e) {
            time = t; maxEnemies = e;
        }
        float time;
        int maxEnemies;
    };

    class MovingEntity: public sf::Sprite {
    public:
        MovingEntity() {
            hitbox.setOrigin(hitbox.getRadius(), hitbox.getRadius());
        }
        sf::Vector2f velocity;
        virtual void update() {
            move(velocity);
            hitbox.setPosition(getPosition());
            velocity.x*=0.8;
            velocity.y*=0.8;
        }
        sf::CircleShape hitbox = sf::CircleShape(10);
    };

    class Object {
    public:
        std::vector <sf::Vector2f> points;
    };

    class CrossingPoint{
    public:
        CrossingPoint(sf::Vector2f v, double a)
        :point(v), angle(a)
        {}

        bool operator<(const CrossingPoint& p) const {
            return angle > p.angle;
        }

        sf::Vector2f point;
        double angle;
    };

    class lightSource: public sf::Vector2f{
    public:
        lightSource(){

        }
        lightSource(sf::Vector2f pos) {
            x=pos.x, y=pos.y;
        }
        lightSource(sf::Vector2f pos, sf::Texture* t, sf::Texture* g, sf::Color color = sf::Color::White) {
            x=pos.x, y=pos.y;
            glow.setTexture(*g);
            glow.setOrigin(glow.getTextureRect().width/2, glow.getTextureRect().height/2);
            glow.setPosition(sf::Vector2f(x, y));
            glow.setColor(color);

            sprite.setTexture(*t);
            sprite.setOrigin(sprite.getTextureRect().width/2, sprite.getTextureRect().height/2);
            sprite.setPosition(sf::Vector2f(x, y));
            //sprite.setColor(sf::Color(255-(255-color.r)/50, 255-(255-color.g)/50, 255-(255-color.b)/50));
            sprite.setColor(color);
        }
        void update() {
            sortPoints();
            sprite.setPosition(x, y);
            glow.setPosition(x, y);
        }
        void sortPoints(){
            std::sort(points.begin(), points.end());
        }
        std::vector <CrossingPoint> points;
        sf::VertexArray shadow = sf::VertexArray(sf::TrianglesFan, 0);
        sf::Sprite glow;
        sf::Sprite sprite;
    };

    class Bullet: public sf::Sprite {
    public:
        Bullet() {

        }
        Bullet(sf::Vector2f p, float speed) {
            setPosition(p);
            v = speed;
        }
        sf::Vector2f velocity;
        void update() {
            move(velocity);
            setRotation(atan2(velocity.y, velocity.x)*180/M_PI);
        }
        void create(sf::Vector2f p, sf::Vector2f d, float speed) {
            setPosition(p);
            v = speed;
            velocity.x = cos (atan2(d.y-p.y, d.x-p.x))*speed;
            velocity.y = sin (atan2(d.y-p.y, d.x-p.x))*speed;
        }
        void setDirection(sf::Vector2f d) {
            velocity.x = cos (atan2(d.y-getPosition().y, d.x-getPosition().x))*v;
            velocity.y = sin (atan2(d.y-getPosition().y, d.x-getPosition().x))*v;
        }
        float damage = 100;
        float knockback = -2;
        bool penetrating = false;
        bool bouncy = false;
        bool friendly = true;
        float v = 5;
    };

    class Weapon {
    public:
        Weapon() {};
        Weapon(MovingEntity* p): parent(p) {};
        void setParent(MovingEntity* p) {parent = p;}
        void setBullet() {
            bullet.damage = damage;
            bullet.knockback = knockback;
            bullet.v = velocity;
        }
        MovingEntity* parent;
        std::string name;
        float reloadTime = 1;
        float fireTime = 1;
        unsigned int magSize = 30;
        unsigned int magAmount = 3;
        unsigned int maxMagAmount = 3;
        unsigned int ammo;
        unsigned int mags;
        int damage = 100;
        float knockback = -2;
        float velocity = 5;
        bool automatic = true;
        sf::Texture texture;
        Bullet bullet;
    };

    class Player: public MovingEntity {
    public:
        Player() {

        }
        void update() {
            angle = atan2(velocity.y, velocity.x);
            speed = sqrt(velocity.x*velocity.x+velocity.y*velocity.y);
            sf::Vector2f m = {std::min(speed, maxSpeed)*cos(angle), std::min(speed, maxSpeed)*sin(angle)};
            move(m);
            ls.x = getPosition().x;
            ls.y = getPosition().y;
            ls.update();
            hitbox.setPosition(getPosition());
        }
        lightSource ls = lightSource(getPosition());
        float health = 100;
        sf::Clock sinceHit;
        sf::Time invincibilityTime = sf::seconds(1);
        float maxSpeed = 10;
        float angle;
        float speed;
        Bullet playerBullet;
        std::vector <Weapon> weapons;
        int currentWeapon = 0;
        void nextWeapon() {
            currentWeapon++;
            if (currentWeapon==weapons.size()) currentWeapon = 0;
        }
        void prevWeapon() {
            currentWeapon--;
            if (currentWeapon==-1) currentWeapon = weapons.size()-1;
        }
        void addWeapon(Weapon w) {
            w.setParent(this);
            weapons.push_back(w);
        }
    };

    class Enemy: public MovingEntity {
    public:
        Enemy(sf::Vector2f p, sf::Texture* t, MovingEntity* ME) {
            setPosition(p);
            setTexture(*t);
            setOrigin(getTextureRect().width/2, getTextureRect().height/2);
            hitbox.setRadius(getGlobalBounds().width/2);
            hitbox.setOrigin(getGlobalBounds().width/2, getGlobalBounds().height/2);
            target = ME;
            healthBar.setFillColor(sf::Color::Red);
            healthBar.setSize(sf::Vector2f(getGlobalBounds().width, 5));
        }
        void update() {
            angle = atan2(destination.y-getPosition().y, destination.x-getPosition().x);
            setRotation(angle*180/M_PI);
            velocity.x += speed* cos(angle);
            velocity.y += speed* sin(angle);
            move(velocity);
            hitbox.setPosition(getPosition());
            velocity.x*=0.8;
            velocity.y*=0.8;
            healthBar.setPosition(sf::Vector2f(getGlobalBounds().left, getGlobalBounds().top-10));
        }
        void onHit (Bullet bullet) {
            health-=bullet.damage;
            velocity.x*=bullet.knockback;
            velocity.y*=bullet.knockback;
        }
        void trackEntity (MovingEntity* ME) {
            target = ME;
        }
        void getDestination() {

        }
        float health = 100;
        float maxSpeed = 5;
        float damage = 10;
        float speed = 1;
        float angle;
        MovingEntity* target;
        sf::Vector2f destination;
        sf::RectangleShape healthBar;
    };

    class PUP: public sf::Sprite {
    public:
        PUP(SHOOTER2D* p) {
            parent = p;
        }
        SHOOTER2D* parent;
        std::string name;
        float duration;
        float time;
        virtual void onPickup() {

        }
    };

    class PUPGiveHealth: public PUP {
    public:
        PUPGiveHealth(SHOOTER2D* p) : PUP(p) {};
        void onPickup() {
            parent->spTadzik.health = 100;
        }
    };

    class PUPGiveAmmo: public PUP {
    public:
        PUPGiveAmmo(SHOOTER2D* p) : PUP(p) {};
        void onPickup() {
            parent->spTadzik.weapons[parent->spTadzik.currentWeapon].mags = parent->spTadzik.weapons[parent->spTadzik.currentWeapon].magAmount;
        }
    };

    void getEnemyPath(Enemy &e) {
        sf::Vector2f rd = e.target->hitbox.getPosition()-e.getPosition();
        sf::Vector2f sp;
        sf::Vector2f sd;
        sf::Vector2f p2;
        sf::Vector2f rp = e.getPosition();
        int furthestObject = -1;
        float rMag;
        float sMag;
        float T1Min=999999;
        float T1, T2;

        for (unsigned int i=0; i<vecWalls.size()-1; i++) {
            for (unsigned int j=0; j<vecWalls[i].points.size(); j++) {
                sp = vecWalls[i].points[j];
                p2 = vecWalls[i].points[(j+1)%vecWalls[i].points.size()];
                sd ={p2.x-sp.x, p2.y-sp.y};
                T2 = (rd.x*(sp.y-rp.y) + rd.y*(rp.x-sp.x))/(sd.x*rd.y - sd.y*rd.x);
                T1 = (sp.x+sd.x*T2-rp.x)/rd.x;
                rMag = sqrt(rd.x*rd.x+rd.y*rd.y);
                sMag = sqrt(sd.x*sd.x+sd.y*sd.y);
                if (T1<T1Min && T1>0 && T1<1 && T2>0 && T2<1 && rd.x/rMag!=sd.x/sMag && rd.y/rMag!=sd.y/sMag) {
                    T1Min=T1;
                    furthestObject = i;
                }
            }
        }
        if (furthestObject == -1) {
            e.destination = e.target->hitbox.getPosition();
            return;
        }
        else {
            float rayAngle = getAngle(e.getPosition(), e.target->hitbox.getPosition());
            float maxAngle = -20;
            float minAngle = 20;
            int pLeft = -1;
            int pRight = -1;
            for (unsigned int i=0; i<vecWalls[furthestObject].points.size(); i++) {
                float tmpAngle = getAngle(e.getPosition(), vecWalls[furthestObject].points[i]);
                tmpAngle-=rayAngle;
                if (tmpAngle<-M_PI) tmpAngle +=2*M_PI;
                if (tmpAngle>=M_PI) tmpAngle -=2*M_PI;
                if (tmpAngle>maxAngle) {
                    pRight = i;
                    maxAngle = tmpAngle;
                }
                if (tmpAngle<minAngle) {
                    pLeft = i;
                    minAngle = tmpAngle;
                }
            }
            if (getMagnitude(e.target->getPosition(), vecWalls[furthestObject].points[pLeft]) < getMagnitude(e.target->getPosition(), vecWalls[furthestObject].points[pRight]))
            {
                e.destination = rotatedPoint(vecWalls[furthestObject].points[pLeft], e.getPosition(), 20);
            }
            else e.destination = rotatedPoint(vecWalls[furthestObject].points[pRight], e.getPosition(), -20);
        }
    }

    double getAngle (sf::Vector2f p1, sf::Vector2f p2) {
        return atan2(p2.y-p1.y, p2.x-p1.x);
    }

    float getMagnitude(sf::Vector2f p1, sf::Vector2f p2) {
        return (p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y);
    }

    void updateShadow(lightSource& ls) {
        ls.points.clear();
        for (unsigned int i=0; i<vecWalls.size(); i++) {
            for (unsigned int j=0; j<vecWalls[i].points.size(); j++){
                ls.points.push_back(getIntersection(ls, vecWalls[i].points[j]));
                ls.points.push_back(getIntersection(ls, rotatedPoint(vecWalls[i].points[j], ls, 0.0001)));
                ls.points.push_back(getIntersection(ls, rotatedPoint(vecWalls[i].points[j], ls, -0.0001)));
                sf::CircleShape c(2);
                c.setOrigin(1, 1);
                c.setPosition(rotatedPoint(vecWalls[i].points[j], ls, 1));
                rDebug.draw(c);
                c.setPosition(rotatedPoint(vecWalls[i].points[j], ls, -1));
                rDebug.draw(c);
            }
        }
        ls.sortPoints();
        ls.shadow.clear();
        sf::Color c = sf::Color(0, 0, 0, 0);
        ls.shadow.append(sf::Vertex(ls, c));
        for (unsigned int i=0; i<ls.points.size(); i++) {
            ls.shadow.append(sf::Vertex(ls.points[i].point, c));
        }
        ls.shadow.append(sf::Vertex(ls.points[0].point, c));
    }

    sf::VertexArray getLine(sf::Vector2f p1, sf::Vector2f p2, sf::Color c = sf::Color::White) {
        sf::VertexArray a (sf::Lines, 2);
        a[0]=p1;
        a[1]=p2;
        return a;
    }

    sf::Vector2f rotatedPoint(sf::Vector2f p, sf::Vector2f center, float d) {
        float a = atan2(p.y-center.y, p.x-center.x);
        sf::Vector2f r;
        r.x = p.x+d*sin(a);
        r.y = p.y-d*cos(a);
        return r;
    }

    CrossingPoint getIntersection (sf::Vector2f rp, sf::Vector2f p2) {
        sf::Vector2f rd = {p2.x-rp.x, p2.y-rp.y};
        sf::Vector2f sp;
        sf::Vector2f sd;
        float rMag;
        float sMag;
        float T1Min=1000;
        float T1, T2;

        for (unsigned int i=0; i<vecWalls.size(); i++) {
            for (unsigned int j=0; j<vecWalls[i].points.size(); j++) {
                sp = vecWalls[i].points[j];
                p2 = vecWalls[i].points[(j+1)%vecWalls[i].points.size()];
                sd ={p2.x-sp.x, p2.y-sp.y};
                T2 = (rd.x*(sp.y-rp.y) + rd.y*(rp.x-sp.x))/(sd.x*rd.y - sd.y*rd.x);
                T1 = (sp.x+sd.x*T2-rp.x)/rd.x;
                rMag = rd.x*rd.x+rd.y*rd.y;
                sMag = sd.x*sd.x+sd.y*sd.y;
                if (T1<T1Min && T1>0 && T2>0 && T2<1 && rd.x/rMag!=sd.x/sMag && rd.y/rMag!=sd.y/sMag)
                    T1Min=T1;
            }
        }
        return CrossingPoint(sf::Vector2f(rp.x+rd.x*T1Min, rp.y+rd.y*T1Min), atan2(rd.y, rd.x));
    }

    virtual void onSceneLoadToMemory() {
        mapa.loadFromFile("files/maps/shooter2D/map1.png");

        deathMessage.setFont(Common::Font::Days_Later);

        texBullet1.loadFromFile("files/textures/shooter2D/bullet1.png");
        tmpBullet.setTexture(texBullet1);
        spTadzik.playerBullet.setTexture(texBullet1);
        tmpBullet.setScale(0.2, 0.2);
        spTadzik.playerBullet.setScale(0.2, 0.2);

        texWall.loadFromFile("files/textures/shooter2D/wall1.png");
        spWall.setTexture(texWall);
        spWall.setOrigin(tileSize/2, tileSize/2);
        texCandle.loadFromFile("files/textures/shooter2D/candle.png");

        texPlayer.loadFromFile("files/textures/shooter2D/player.png");
        spTadzik.setTexture(texPlayer);
        Utils::setOriginInCenter(spTadzik);

        texHealthBar.loadFromFile("files/textures/shooter2D/healthBar.png");
        texHealthFrame.loadFromFile("files/textures/shooter2D/healthFrame.png");

        texShadow.loadFromFile("files/textures/shooter2D/shadow.png");

        spTadzik.ls.glow.setTexture(texShadow);
        Utils::setOriginInCenter(spTadzik.ls.glow);

        loadMap();
        loadWaves();

        rTexture.create(window->getSize().x, window->getSize().y);
        rDebug.create(window->getSize().x, window->getSize().y);
        rLines.create(window->getSize().x, window->getSize().y);
        rShadows.create(window->getSize().x, window->getSize().y);
        rHelp.create(window->getSize().x, window->getSize().y);

        Object tmpObject;
        tmpObject.points.push_back(sf::Vector2f(tileSize, tileSize));
        tmpObject.points.push_back(sf::Vector2f(window->getSize().x-tileSize, tileSize));
        tmpObject.points.push_back(sf::Vector2f(window->getSize().x-tileSize, window->getSize().y-tileSize));
        tmpObject.points.push_back(sf::Vector2f(tileSize, window->getSize().y-tileSize));

        vecWalls.push_back(tmpObject);

        rLines.clear(sf::Color(0, 0, 0, 0));
        for (unsigned int i=0; i<vecWalls.size(); i++) {
            for (unsigned int j=0; j<vecWalls[i].points.size(); j++) {
                rLines.draw(getLine(vecWalls[i].points[j], vecWalls[i].points[(j+1)%vecWalls[i].points.size()]));
            }
        }
        rLines.display();

        texHud.loadFromFile("files/textures/shooter2D/hud.png");
        hud.create(window->getSize().x, window->getSize().y);
        hud.healthBar.setTexture(texHealthBar);
        hud.healthFrame.setTexture(texHealthFrame);
        hud.frame.setTexture(texHud);
        hud.setFont(&Common::Font::Days_Later, sf::Color::Black);

        texCrosshair.loadFromFile("files/textures/shooter2D/crosshair.png");
        spCrosshair.setTexture(texCrosshair);
        Utils::setOriginInCenter(spCrosshair);

        texEnemy1.loadFromFile("files/textures/shooter2D/enemy1.png");

        loadWeapons();
        spTadzik.addWeapon(vecWeapons[0]);
        spTadzik.addWeapon(vecWeapons[1]);
        spTadzik.addWeapon(vecWeapons[2]);
        spTadzik.addWeapon(vecWeapons[3]);
        spTadzik.currentWeapon = 0;


        texPUPGiveHealth.loadFromFile("files/textures/shooter2D/texPUPGiveHealth.png");
        tmpPUPGiveHealth.setTexture(texPUPGiveHealth);
        Utils::setOriginInCenter(tmpPUPGiveHealth);

        texPUPGiveAmmo.loadFromFile("files/textures/shooter2D/texPUPGiveAmmo.png");
        tmpPUPGiveAmmo.setTexture(texPUPGiveAmmo);
        Utils::setOriginInCenter(tmpPUPGiveAmmo);
    }

    virtual void onSceneActivate() {
        window->setMouseCursorVisible(false);
    }

    void loadMap() {
        bool t[mapa.getSize().x][mapa.getSize().y];
        sf::Color objectColor = sf::Color(255, 255, 255);
        for (unsigned int i=0; i<mapa.getSize().x; i++) {
            for (unsigned int j=0; j<mapa.getSize().y; j++) {
                if (mapa.getPixel(i, j)==objectColor && t[i][j]==0) {
                    Object tmpWall;
                    sf::Vector2i pocz = {i, j};
                    sf::Vector2i on = pocz;
                    std::vector <sf::Vector2i> sprites;
                    sprites.push_back(on);
                    int d = 0;
                    int dPrev = 3;
                    bool did = false;
                    do {
                        bool repeated = false;
                        if (mapa.getPixel(on.x, on.y) == objectColor) {
                            for (unsigned int i=0; i<sprites.size(); i++)
                                if (sprites[i]==on)
                                    repeated = true;
                            if (!repeated) sprites.push_back(on);
                        }
                        if (did) {
                            dPrev = d;
                            d=(d+3)%4;
                            did = false;
                        }
                        if (d==0) {
                            if (mapa.getPixel(on.x, on.y) == objectColor || mapa.getPixel(on.x, on.y-1) == objectColor) {
                                if (d!=dPrev && abs(d-dPrev)!=2) tmpWall.points.push_back(sf::Vector2f(on.x*tileSize, on.y*tileSize));
                                on.x++;
                                did = true;
                                t[on.x][on.y] = 1;
                            }
                        }
                        else if (d==1) {
                            if (mapa.getPixel(on.x, on.y) == objectColor || mapa.getPixel(on.x-1, on.y) == objectColor) {
                                if (d!=dPrev && abs(d-dPrev)!=2) tmpWall.points.push_back(sf::Vector2f(on.x*tileSize, on.y*tileSize));
                                on.y++;
                                did = true;
                                t[on.x][on.y] = 1;
                            }
                        }
                        else if (d==2) {
                            if (mapa.getPixel(on.x-1, on.y) == objectColor || mapa.getPixel(on.x-1, on.y-1) == objectColor) {
                                if (d!=dPrev && abs(d-dPrev)!=2) tmpWall.points.push_back(sf::Vector2f(on.x*tileSize, on.y*tileSize));
                                on.x--;
                                did = true;
                                t[on.x][on.y] = 1;
                            }
                        }
                        else if (d==3) {
                            if (mapa.getPixel(on.x, on.y-1) == objectColor || mapa.getPixel(on.x-1, on.y-1) == objectColor) {
                                if (d!=dPrev && abs(d-dPrev)!=2) tmpWall.points.push_back(sf::Vector2f(on.x*tileSize, on.y*tileSize));
                                on.y--;
                                did = true;
                                t[on.x][on.y] = 1;
                            }
                        }
                        if (!did) d=(d+1)%4;
                    }
                    while (on!=pocz);
                    for (auto a:sprites) {
                        spWall.setPosition((a.x+0.5)*tileSize, (a.y+0.5)*tileSize);
                        vecSprites.push_back(spWall);
                    }
                    vecWalls.push_back(tmpWall);
                }
                else if (mapa.getPixel(i, j) == sf::Color(0, 0, 0)) {
                    spWall.setPosition((i+0.5)*tileSize, (j+0.5)*tileSize);
                    vecSprites.push_back(spWall);
                }
                else if (mapa.getPixel(i, j) == sf::Color(0, 0, 255)) {
                    spTadzik.setPosition(i*tileSize, j*tileSize);
                }
            }
        }
    }

    void loadWeapons() {
        std::ifstream file;
        std::string texDir;
        Weapon tmpWeapon;
        tmpWeapon.bullet.setTexture(texBullet1);
        tmpWeapon.bullet.setScale(0.2, 0.2);
        tmpWeapon.bullet.setOrigin(tmpWeapon.bullet.getTextureRect().width/2, tmpWeapon.bullet.getTextureRect().height/2);
        file.open("files/resource/shooter2D/weapons.dat");
        while (!file.eof()) {
            file >> tmpWeapon.name;
            file >> texDir;
            tmpWeapon.texture.loadFromFile(texDir);
            file >> tmpWeapon.damage;
            file >> tmpWeapon.magSize; tmpWeapon.ammo = tmpWeapon.magSize;
            file >> tmpWeapon.magAmount; tmpWeapon.mags = tmpWeapon.magAmount;
            file >> tmpWeapon.automatic;
            file >> tmpWeapon.reloadTime;
            file >> tmpWeapon.fireTime;
            file >> tmpWeapon.velocity;
            file >> tmpWeapon.knockback;
            tmpWeapon.setBullet();
            vecWeapons.push_back(tmpWeapon);
        }
    }

    void loadWaves() {
        std::ifstream file;
        file.open("files/resource/shooter2D/waves.dat");
        Wave tmpWave(0, 0);
        while (!file.eof()) {
            file >> tmpWave.time;
            file >> tmpWave.maxEnemies;
            vecWaves.push_back(tmpWave);
        }
    }

    void handleCollision(MovingEntity& s1, std::vector <sf::Sprite>& walls) {
        for (unsigned int i=0; i<walls.size(); ++i) {
            sf::FloatRect intersection;
            if (Collision::PixelPerfectTest(walls[i], s1) && s1.getGlobalBounds().intersects(walls[i].getGlobalBounds(), intersection)) {
                sf::Vector2f direction = walls[i].getPosition() - s1.getPosition();
                sf::Vector2f offset;
                // X collision
                if (abs(direction.x) > abs(direction.y))
                    offset.x = ((direction.x<0)?-1:1)*intersection.width;
                // Y collision
                if (abs(direction.x) < abs(direction.y))
                    offset.y = ((direction.y<0)?-1:1)*intersection.height;
                s1.velocity.x -= offset.x * abs(offset.x)/50.0f;
                s1.velocity.y -= offset.y * abs(offset.y)/50.0f;
            }
        }
    }

    void handleEntityCollision (MovingEntity& e1, MovingEntity &e2, double knockback) {
        sf::FloatRect intersection;
        if (e1.hitbox.getGlobalBounds().intersects(e2.hitbox.getGlobalBounds(), intersection)) {
            sf::Vector2f direction = e2.getPosition() - e1.getPosition();
            sf::Vector2f offset;
            offset.x = ((direction.x<0)?-1:1)*intersection.width;
            offset.y = ((direction.y<0)?-1:1)*intersection.height;
            offset.x*=knockback;
            offset.y*=knockback;
            e1.velocity -= offset;
            e2.velocity += offset;
        }
    }

    void gameOver() {
        isDead = true;
        deathMessage.setString("YOU SUCK");
    }

    void deliverEvent(sf::Event& event){
        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left && !spTadzik.weapons[spTadzik.currentWeapon].automatic) {
                shoot();


            }
            if (event.mouseButton.button == sf::Mouse::Right) {
                vecLights.push_back(lightSource(sf::Vector2f(sf::Mouse::getPosition(*window)), &texCandle, &texShadow, Utils::randColor()));
                updateShadow(vecLights[vecLights.size()-1]);
            }
        }
        if (event.type == sf::Event::MouseWheelMoved) {
            spTadzik.currentWeapon-=event.mouseWheel.delta;
            while (spTadzik.currentWeapon<0) spTadzik.currentWeapon += spTadzik.weapons.size();
            spTadzik.currentWeapon = spTadzik.currentWeapon%spTadzik.weapons.size();
            isReloading = false;
            if (spTadzik.weapons[spTadzik.currentWeapon].ammo == 0) reload();
        }
    }

    void reload() {
        if (spTadzik.weapons[spTadzik.currentWeapon].ammo < spTadzik.weapons[spTadzik.currentWeapon].magSize && !isReloading && spTadzik.weapons[spTadzik.currentWeapon].mags > 0) {
            isReloading = true;
            reloadFor.restart();
        }
    }

    void shoot() {
        if (lastShot.getElapsedTime().asMilliseconds() > spTadzik.weapons[spTadzik.currentWeapon].fireTime && spTadzik.weapons[spTadzik.currentWeapon].ammo>0 && !isReloading) {
            lastShot.restart();
            spTadzik.weapons[spTadzik.currentWeapon].ammo--;
            tmpBullet = spTadzik.weapons[spTadzik.currentWeapon].bullet;
            tmpBullet.setPosition(spTadzik.getPosition());
            tmpBullet.setDirection(sf::Vector2f(sf::Mouse::getPosition(*window)));
            vecBullets.push_back(tmpBullet);
            if (spTadzik.weapons[spTadzik.currentWeapon].ammo == 0) reload();
        }
    }

    void updateEnemies() {
        for (int i=vecEnemies.size()-1; i>=0; i--) {
            getEnemyPath(vecEnemies[i]);
            sf::VertexArray a (sf::LinesStrip, 3);
            a[0]=sf::Vertex(vecEnemies[i].getPosition(), sf::Color::Red);
            a[1]=sf::Vertex(vecEnemies[i].destination, sf::Color::Red);
            a[2]=sf::Vertex(vecEnemies[i].target->hitbox.getPosition(), sf::Color::Red);
            rDebug.draw(a);
            handleCollision(vecEnemies[i], vecSprites);
            for (int j=vecBullets.size()-1; j>=0; j--) {
                if (Collision::BoundingBoxTest(vecEnemies[i], vecBullets[j]) && vecBullets[j].friendly) {
                    vecEnemies[i].onHit(vecBullets[j]);
                    vecBullets.erase(vecBullets.begin()+j);
                    vecEnemies[i].healthBar.setScale(vecEnemies[i].health/100.0, 1);
                    if (vecEnemies[i].health<=0) {
                        if (Utils::chance(0.33)) {
                            tmpPUPGiveHealth.setPosition(vecEnemies[i].getPosition());
                            vecPUPGiveHealth.push_back(tmpPUPGiveHealth);
                        }
                        else {
                            tmpPUPGiveAmmo.setPosition(vecEnemies[i].getPosition());
                            vecPUPGiveAmmo.push_back(tmpPUPGiveAmmo);
                        }
                        vecEnemies.erase(vecEnemies.begin()+i);
                        score++;
                        break;
                    }
                }
            }
        }
        for (unsigned int i=0; i+1<vecEnemies.size(); i++) {
            for (unsigned int j=i+1; j<vecEnemies.size(); j++) {
                handleEntityCollision(vecEnemies[i], vecEnemies[j], 0.02);
            }
        }
        if (clock.getElapsedTime().asSeconds()>vecWaves[currentWave].time) {
            sf::Image img;
            img = rHelp.getTexture().copyToImage();
            for (int i=0; i<vecWaves[currentWave].maxEnemies; i++) {
                sf::Vector2i t = Utils::randVector2i(sf::IntRect(10, 10, 1260, 700));
                if (img.getPixel(t.x, t.y) == sf::Color::Black)
                    vecEnemies.push_back(Enemy(sf::Vector2f(t), &texEnemy1, &spTadzik));
            }
            clock.restart();
            currentWave++;
            currentWave=currentWave%vecWaves.size();
        }
    }

    void updateHUD() {
        hud.tScore.setString(Utils::stringify(score));
        hud.tAmmo.setString(Utils::stringify(spTadzik.weapons[spTadzik.currentWeapon].ammo));
        hud.tAllAmmo.setString(Utils::stringify(spTadzik.weapons[spTadzik.currentWeapon].mags));
        hud.healthBar.setScale(spTadzik.health/100, 1);
        hud.activeWeapon.setTexture(spTadzik.weapons[spTadzik.currentWeapon].texture);
        hud.clear(sf::Color(0, 0, 0, 0));
        if (isReloading) {
            hud.reloading.setScale(1-reloadFor.getElapsedTime().asMilliseconds()/spTadzik.weapons[spTadzik.currentWeapon].reloadTime, 1);
            hud.reloading.setPosition(spTadzik.getGlobalBounds().left, spTadzik.getPosition().y);
            hud.draw(hud.reloading);
        }
        hud.update();
    }

    virtual bool onConsoleUpdate(std::vector<std::string> args){
        if (args[0] == "debug") {
            debug = !debug;
        }
        else if (args[0] == "gib") {
            for (unsigned int i=0; i<spTadzik.weapons.size(); i++) {
                spTadzik.weapons[i].ammo = spTadzik.weapons[i].magSize;
                spTadzik.weapons[i].mags = spTadzik.weapons[i].magAmount;
                spTadzik.health = 100;
            }
        }
        else if (args.size()==2 && args[0] == "spawn") {
            for (int i=0; i<atoi(args[1].c_str()); i++)
                vecEnemies.push_back(Enemy(sf::Vector2f(Utils::randInt(10, window->getSize().x-10), Utils::randInt(10, window->getSize().y)), &texEnemy1, &spTadzik));
        }
        else if (args[0] == "killall") {
            vecEnemies.clear();
        }
        else if (args[0] == "stopspawning") {
            vecEnemies.clear();
            currentWave = 0;
            vecWaves[0].time = 1000000;
        }
        else if (args[0] == "lightsoff") {
            vecLights.clear();
        }
        else
            return false;
        return true;
    }

    virtual void getKeyboardStuff() {
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) && -spTadzik.velocity.x < spTadzik.maxSpeed)
            spTadzik.velocity.x -= acceleration;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) && -spTadzik.velocity.y < spTadzik.maxSpeed)
            spTadzik.velocity.y -= acceleration;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) && spTadzik.velocity.y < spTadzik.maxSpeed)
            spTadzik.velocity.y += acceleration;
        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) && spTadzik.velocity.x < spTadzik.maxSpeed)
            spTadzik.velocity.x += acceleration;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) spTadzik.health-=10;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) reload();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
            vecEnemies.push_back(Enemy(sf::Vector2f(Utils::randInt(10, window->getSize().x-10), Utils::randInt(10, window->getSize().y)), &texEnemy1, &spTadzik));
    }

    virtual void draw(double deltaTime) {
        /// INPUT MYSZK�
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && spTadzik.weapons[spTadzik.currentWeapon].automatic) {
            shoot();
        }

        /// INPUT Z KLAWIATURY
        getKeyboardStuff();

        /// RELOADING
        if (isReloading) {
            if (reloadFor.getElapsedTime().asMilliseconds()>spTadzik.weapons[spTadzik.currentWeapon].reloadTime) {
                isReloading = false;
                spTadzik.weapons[spTadzik.currentWeapon].ammo = spTadzik.weapons[spTadzik.currentWeapon].magSize;
                spTadzik.weapons[spTadzik.currentWeapon].mags--;
            }
        }

        /// OGARNIANIE KOLIZJI
        for (unsigned int i=0; i<vecEnemies.size(); i++) {
            if (Collision::PixelPerfectTest(spTadzik, vecEnemies[i])) {
                handleEntityCollision(spTadzik, vecEnemies[i], 0.5);
                if (spTadzik.sinceHit.getElapsedTime() > spTadzik.invincibilityTime) {
                    spTadzik.health-=vecEnemies[i].damage;
                    spTadzik.sinceHit.restart();
                }
            }
        }

        if (spTadzik.health<0) gameOver();

        spTadzik.update();
        if (debug) spTadzik.setPosition(sf::Vector2f(sf::Mouse::getPosition(*window)));
        handleCollision(spTadzik, vecSprites);

        spTadzik.velocity.x*=0.8;
        spTadzik.velocity.y*=0.8;
        spTadzik.setRotation(atan2(sf::Mouse::getPosition(*window).y-spTadzik.getPosition().y, sf::Mouse::getPosition(*window).x-spTadzik.getPosition().x)*180/M_PI);

        updateShadow(spTadzik.ls);

        window->clear(sf::Color(255-20*(clock.getElapsedTime().asSeconds()/vecWaves[currentWave].time),
                                255, 255));


        /// OGARNIANIE PRZECIWNIK�W
        for (unsigned int i=0; i<vecEnemies.size(); i++) {
            vecEnemies[i].update();
            rDebug.draw(vecEnemies[i].hitbox);
            window->draw(vecEnemies[i]);
            window->draw(vecEnemies[i].healthBar);
        }
        updateEnemies();

        /// OGARNIANIE POCISK�W
        if (vecBullets.size()>0) {
            for (int i=vecBullets.size()-1; i>=0; i--) {
                vecBullets[i].update();
                window->draw(vecBullets[i]);
                for (unsigned int j=0; j<vecSprites.size(); j++) {
                    if (Collision::BoundingBoxTest(vecBullets[i], vecSprites[j])) {
                        vecBullets.erase(vecBullets.begin()+i);
                        break;
                    }
                }
            }
        }

        /// OGARNIANIE POWERUPOW
        for (int i=vecPUPGiveHealth.size()-1; i>=0; --i) {
            window->draw(vecPUPGiveHealth[i]);
            if (Collision::BoundingBoxTest(spTadzik, vecPUPGiveHealth[i])) {
                vecPUPGiveHealth[i].onPickup();
                vecPUPGiveHealth.erase(vecPUPGiveHealth.begin()+i);
            }
        }
        for (int i=vecPUPGiveAmmo.size()-1; i>=0; --i) {
            window->draw(vecPUPGiveAmmo[i]);
            if (Collision::BoundingBoxTest(spTadzik, vecPUPGiveAmmo[i])) {
                vecPUPGiveAmmo[i].onPickup();
                vecPUPGiveAmmo.erase(vecPUPGiveAmmo.begin()+i);
            }
        }

        /// TEMPORARY LIGHTNING
        rTexture.clear(sf::Color(0, 0, 0, 255));
        rHelp.clear(sf::Color(0, 0, 0, 255));
        for (unsigned int i = 0; i<vecLights.size(); i++)
            rHelp.draw(vecLights[i].sprite, sf::BlendAdd);

        rShadows.clear(sf::Color(0, 0, 0));
        rTexture.draw(spTadzik.ls.shadow, sf::BlendNone);
        rTexture.display();
        rShadows.draw(spTadzik.ls.glow, sf::BlendAdd);
        rShadows.draw(sf::Sprite(rTexture.getTexture()));
        rShadows.display();
        rHelp.draw(sf::Sprite(rShadows.getTexture()), sf::BlendAdd);
        for (unsigned int i=0; i<vecLights.size(); i++) {
            rTexture.clear(sf::Color(0, 0, 0, 255));
            rShadows.clear(sf::Color(0, 0, 0, 255));
            rTexture.draw(vecLights[i].shadow, sf::BlendNone);
            rTexture.display();
            rShadows.draw(vecLights[i].glow, sf::BlendAdd);
            rShadows.draw(sf::Sprite(rTexture.getTexture()));
            rShadows.display();
            rHelp.draw(sf::Sprite(rShadows.getTexture()), sf::BlendAdd);
        }
        rHelp.display();

        window->draw(sf::Sprite(rHelp.getTexture()), sf::BlendMultiply);
        window->draw(sf::Sprite(rLines.getTexture()));

        /// DEBUG
        if (debug) {
            for (unsigned int i=0; i<spTadzik.ls.points.size(); i++) {
                sf::CircleShape c(2);
                c.setOrigin(1, 1);
                c.setFillColor(sf::Color::Green);
                c.setPosition(spTadzik.ls.points[i].point);
                rDebug.draw(c);
            }
            rDebug.display();
            window->draw(sf::Sprite(rDebug.getTexture()));
        }
        rDebug.clear(sf::Color(0, 0, 0, 0));


        window->draw(spTadzik);

        updateHUD();
        window->draw(sf::Sprite(hud.getTexture()));
        spCrosshair.setPosition(sf::Vector2f(sf::Mouse::getPosition(*window)));
        window->draw(spCrosshair);
        if (isDead) {
            window->draw(deathMessage);
        }
    }

protected:
    sf::Texture texBackground;
    sf::Texture texWall;
    sf::Texture texPlayer;
    sf::Texture texHealthBar;
    sf::Texture texHealthFrame;
    sf::Texture texBullet1;
    sf::Texture texCandle;
    sf::Texture texEnemy1;
    sf::Texture texCrosshair;
    sf::Texture texHud;
    sf::Texture texShadow;
    sf::Texture texPUPGiveHealth;
    sf::Texture texPUPGiveAmmo;

    sf::Sprite spCrosshair;

    sf::Image mapa;
    sf::Text deathMessage;
    sf::RenderTexture rTexture;
    sf::RenderTexture rDebug;
    sf::RenderTexture rGUI;
    sf::RenderTexture rLines;
    sf::RenderTexture rShadows;
    sf::RenderTexture rHelp;
    HUD hud;

    Player spTadzik;
    sf::Sprite spWall;

    Bullet tmpBullet;

    PUPGiveHealth tmpPUPGiveHealth = PUPGiveHealth(this);
    PUPGiveAmmo tmpPUPGiveAmmo = PUPGiveAmmo(this);

    std::vector <lightSource> vecLights;

    std::vector <Object> vecWalls;

    std::vector <Enemy> vecEnemies;

    std::vector <Bullet> vecBullets;

    std::vector <sf::Sprite> vecSprites;

    std::vector <Weapon> vecWeapons;

    std::vector <Wave> vecWaves;

    std::vector <PUPGiveHealth> vecPUPGiveHealth;
    std::vector <PUPGiveAmmo> vecPUPGiveAmmo;

    float acceleration = 2;
    int tileSize = 20;
    int score = 0;

    sf::Clock lastShot;
    sf::Clock reloadFor;
    sf::Clock clock;

    int currentWave = 0;

    bool isReloading = false;
    bool isDead = false;

    bool debug = false;
};

#endif //SHOOTER2D
