//Copyright Haithem Gazzah & Matthieu Ospici


#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include <cmath>
#define MAX_SIZE 10
using namespace std;

class GameState;

typedef enum act_t {MOVE, THROW} act_t;
typedef enum en_t {WIZARD, OPPONENT_WIZARD ,SNAFFLE,BLUDGER} en_t;




struct Coordinates
{
public:
  float x;
  float y;
  Coordinates (float _x, float _y):x(_x),y(_y){}
  Coordinates(){}
  static Coordinates vector_from_points(Coordinates c1,Coordinates c2) 
  { 
    Coordinates c; 
    c.x = c2.x - c1.x; 
    c.y = c2.y - c1.y; 
    return c; 
  }

  //thanks to magus, http://files.magusgeek.com/csb/csb.html
  Coordinates closest(const Coordinates &a,  const Coordinates &b) const
  {
    float da = b.y - a.y;
    float db = a.x - b.x;
    float c1 = da*a.x + db*a.y;
    float c2 = -db*this->x + da*this->y;
    float det = da*da + db*db;
    float cx = 0;
    float cy = 0;

    if (det != 0)
      {
        cx = (da*c1 - db*c2) / det;
        cy = (da*c2 + db*c1) / det;
      }
    else
      {
        cx = this->x;
        cy = this->y;
      }

    return Coordinates(cx, cy);
}

  
  static float comp_dist(const Coordinates &c1,const Coordinates &c2) 
  { 
    return sqrt(pow(c1.x-c2.x,2) + pow(c1.y - c2.y,2)); 
  } 


  static float comp_dist2(const Coordinates &c1,const Coordinates &c2) 
  { 
    return (c1.x-c2.x)*(c1.x-c2.x) + (c1.y - c2.y)*(c1.y - c2.y); 
  } 
 
  //return normalized vector from 2 points 
  static Coordinates compute_norm_vect(const Coordinates &c1,const Coordinates &c2) 
  { 
    Coordinates cord = vector_from_points(c1,c2); 
    float norm_v = comp_dist(c1,c2); 
    cord.x = (float)cord.x/norm_v; 
    cord.y = (float)cord.y/norm_v; 
    return cord; 
  } 
	 


  
};


class Action
{	
public:
  act_t type;
  Coordinates c;
  int arg;//thrust for move, power for throw, id for spell
  void print()
  {
    switch(this->type)
      {
      case MOVE:
	cout<<"MOVE "<<this->c.x<<" "<<this->c.y<<" "<<this->arg<<endl;
	break;
      case THROW:
	cout<<"THROW "<<this->c.x<<" "<<this->c.y<<" "<<this->arg<<endl;
	break;
      }
  }
};

class Entity;

struct Collision
{
public:
  Collision():a(NULL),b(NULL),time(0){}

  Collision(const Entity* a_, const Entity* b_,float t_):a(a_),b(b_),time(t_){}
  //private:
  const Entity* a;
  const Entity *b;
  float time;
};




class Entity
{
public:
  Coordinates c;
  int id;
  int vx;
  int vy;
  float weight;
  float friction;
  float radius;
  int state;
  en_t type;
  Entity()
  {
  }
  void setProperties(int _id, en_t _type, int _x, int _y, int _vx, int _vy,int _state)
  {
    this->id = _id;
    this->c.x=_x;
    this->c.y=_y;
    this->vx=_vx;
    this->vy=_vy;
    this->type=_type;
    this->state=_state;
    switch(_type)
      {
      case OPPONENT_WIZARD:
      case BLUDGER:
	
      case WIZARD:
	this->weight = 1;
	this->friction = 0.75;
	this->radius = 400;
	break;
      case SNAFFLE:
	this->weight= 0.5;
	this->friction=0.75;
	this->radius = 150;
	break;
      }

  }
	
  int get_closest_entity(en_t targetType,GameState *game_state,float *out_dist)
  {
    //TODO
  }
	
  void print()
  {
    cerr<<"ID = "<<id<<" X = "<<c.x<<" Y ="<<c.y<<" vX="<<vx<<" vY="<<vy<<endl;

  }


  float comp_dist_to_2(const Entity& e) const
  {
    return Coordinates::comp_dist2(c,e.c);
  }

  
  bool collision(const Entity &u,Collision &col_out) const
  {
    // Distance carré
    float dist = comp_dist_to_2(u);

    // Somme des rayons au carré
    float sr = (this->radius + u.radius)*(this->radius + u.radius);

    // On prend tout au carré pour éviter d'avoir à appeler un sqrt inutilement. C'est mieux pour les performances

    if (dist < sr) {
        // Les objets sont déjà l'un sur l'autre. On a donc une collision immédiate
      //        return Collision(this, u, 0.0);
	col_out = Collision(this, &u, 0.0);
	return true;
    }

    // Optimisation. Les objets ont la même vitesse ils ne pourront jamais se rentrer dedans
    if (this->vx == u.vx && this->vy == u.vy) {
      //        return null;
	return false;
    }

    // On se met dans le référentiel de u. u est donc immobile et se trouve sur le point (0,0) après ça
    float lx = this->c.x - u.c.x;
    float ly = this->c.y - u.c.y;
    Coordinates myp = Coordinates(lx, ly);
    float lvx = this->vx - u.vx;
    float lvy = this->vy - u.vy;
    Coordinates up = Coordinates(0, 0);

    // On cherche le point le plus proche de u (qui est donc en (0,0)) sur la droite décrite par notre vecteur de vitesse
    Coordinates p = up.closest(myp, Coordinates(lx + lvx, ly + lvy));

    // Distance au carré entre u et le point le plus proche sur la droite décrite par notre vecteur de vitesse
    //    float pdist = up.comp_dist_to_2(p);
    float pdist = Coordinates::comp_dist2(up,p);

    // Distance au carré entre nous et ce point
    //    float mypdist = myp.comp_dist_to_2(p);
    float mypdist = Coordinates::comp_dist2(myp,p);
    // Si la distance entre u et cette droite est inférieur à la somme des rayons, alors il y a possibilité de collision
    if (pdist < sr) {
        // Notre vitesse sur la droite
        float length = sqrt(lvx*lvx + lvy*lvy);

        // On déplace le point sur la droite pour trouver le point d'impact
        float backdist = sqrt(sr - pdist);
        p.x = p.x - backdist * (lvx / length);
        p.y = p.y - backdist * (lvy / length);

        // Si le point s'est éloigné de nous par rapport à avant, c'est que notre vitesse ne va pas dans le bon sens
        if ( Coordinates::comp_dist2(myp,p)> mypdist) {
	  //            return null;
	  return false;
        }
	pdist = Coordinates::comp_dist(p,myp);
	  //pdist = p.distance(myp);

        // Le point d'impact est plus loin que ce qu'on peut parcourir en un seul tour
        if (pdist > length) {
            return false;
        }

        // Temps nécessaire pour atteindre le point d'impact
        float t = pdist / length;

	//        return Collision(this, u, t);
	col_out = Collision(this, &u, t);
	return true;
    }

    //    return null;
    return false;
}
};





class GameState 
{
public:
  Entity list_ent[13];
  int num_ents;
  int list_sna[8];
  int num_sna;
  int list_wiz[2]; //id des sorcier
  int list_op[2]; //id des oponant
  static int team_id;

  inline const Entity& get_sna(int sna_id) const
  {
    return list_ent[list_sna[sna_id]];
  }

  inline const Entity& get_wiz(int wiz_id) const
  {
    return list_ent[list_wiz[wiz_id]];
  }
  
  GameState()
  {
  }
  void create_entity_from_input()
  {
    cin >> num_ents; cin.ignore();
    int wiz_count=0;
    int op_count=0;
    int sna_count=0;
    for (int i = 0; i < num_ents; i++) {
      int entityId; // entity identifier
      string entityType; // "WIZARD", "OPPONENT_WIZARD" or "SNAFFLE" (or "BLUDGER" after first league)
      int x; // position
      int y; // position
      int vx; // velocity
      int vy; // velocity
      int state; // 1 if the wizard is holding a Snaffle, 0 otherwise
      cin >> entityId >> entityType >> x >> y >> vx >> vy >> state; cin.ignore();
   
      if(entityType.compare("SNAFFLE") == 0 )
	{
	  list_ent[i].setProperties(entityId, SNAFFLE ,x ,y ,vx ,vy ,state);
	  list_sna[sna_count++]=i;
	}
      else if (entityType.compare("WIZARD") == 0)
	{
	  list_ent[i].setProperties(entityId, WIZARD ,x ,y ,vx ,vy ,state);
	  list_wiz[wiz_count++]=i;
	}
      else
	{
	  list_ent[i].setProperties(entityId, OPPONENT_WIZARD ,x ,y ,vx ,vy ,state);
	  list_op[op_count++]=i;
	}
    }
    num_sna = sna_count;
  }
  void print_entities()
  {
    for(int i=0; i<num_ents; i++)
      list_ent[i].print();

  }


  float eval() //evaluation fct, compute the score of the board
  {
    //first approach, good score if snafles are at the right side
    // if 0 you need to score on the right of the map, if 1 you need to score on the left

    int num_sna = 0;
    if(team_id == 0)
      {
	for(int i = 0;i<num_sna;++i)
	  {
	    if(get_sna(i).c.x >8000) ++num_sna;
	  }
      }
    else
      {
	for(int i = 0;i<num_sna;++i)
	  {
	    if(get_sna(i).c.x <8000) ++num_sna;
	  }
      }


    return num_sna;
    
  }
	
};


class Simulator 
{
public:
  void predict_entity_state(Entity *entity, Action *action, int *out_x, int *out_y, int *out_vx,int *out_vy)
  {
    switch(action->type)
      {
      case MOVE:
	Coordinates cn = Coordinates::compute_norm_vect(entity->c,action->c);
	cn.x = cn.x*(float)action->arg;
	cn.y = cn.y*(float)action->arg;
				
	float vx_n = entity->vx + cn.x;
	float vy_n = entity->vy + cn.y;
				
	*out_x = round(entity->c.x+vx_n);
	*out_y = round(entity->c.y+vy_n);
				
	*out_vx = round(vx_n*entity->friction/entity->weight);
	*out_vy = round(vy_n*entity->friction/entity->weight);
      }
  }
  GameState * predict_state(GameState* game_state)
  {
  }
};
struct gamestate_queue
{
	
  char size;
  GameState * g_array[MAX_SIZE];
  char front = 0;
  char rear = -1 ;
	
  void insert(GameState *gameState)
  {
    if(rear==(MAX_SIZE-1))
      {
	delete g_array[front];
	front = (front+1)%MAX_SIZE;
			
      }
    rear = (rear+1)%MAX_SIZE;
    g_array[rear] = gameState;
  }
  GameState* get(char offset)
  {
    if(rear==-1) return NULL;
    if(offset ==0)return g_array[rear];
    size=(MAX_SIZE-front+rear)%MAX_SIZE;
    if (offset>size) return NULL;
    char index = (size-offset+front)%MAX_SIZE;
    return g_array[index];
  }
};
class IA_engine
{
  Simulator simulator;
  Action get_action(int entity_index,GameState *game_state,gamestate_queue *history)
  {
    //return Action(MOVE, 
  }
	
};

/**
 * Grab Snaffles and try to throw them through the opponent's goal!
 * Move towards a Snaffle and use your team id to determine where you need to throw it.
 **/
int GameState::team_id =0;
int main()
{
  // if 0 you need to score on the right of the map, if 1 you need to score on the left
  cin >> GameState::team_id; cin.ignore();

  IA_engine ia;
  gamestate_queue history;
  GameState *game_state;
  // game loop
  while (1) {
    game_state = new GameState();
    game_state->create_entity_from_input();
    //game_state->print_entities();

    Entity wiz = game_state->get_wiz(0);

    Action act;
    act.type = MOVE;
    act.c = wiz.c;
    act.arg = 150;
    for (int i = 0; i < 2; i++)
      {
	//gamestate
	act.print();
	Collision col_out;
	if(wiz.collision(game_state->get_wiz(1),col_out))
	  {
	    cerr << col_out.time << endl;
	  }

      }


    history.insert(game_state);
  }
}
