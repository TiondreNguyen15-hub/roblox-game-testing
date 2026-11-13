#pragma once
#include <array>
#include <string>
#include <vector>
namespace apex {
struct Vec3 {float x,y,z;};
struct Vec2 {float x,y;};
struct Entity {int id,team;float health;bool visible;std::string name;Vec3 position;std::array<Vec3,15> bones;};
class Simulation {public: void update(double time,int count,int scene=0);const std::vector<Entity>& entities()const{return entities_;}private:std::vector<Entity> entities_;};
bool project(Vec3 world,float width,float height,Vec2& screen);
float distance(Vec3 p);
}
