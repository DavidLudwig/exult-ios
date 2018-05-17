/*
 *  objs.h - Game objects.
 *
 *  Copyright (C) 1998-1999  Jeffrey S. Freedman
 *  Copyright (C) 2000-2013  The Exult Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef OBJS_H
#define OBJS_H

#include <string>   // STL string
#include <set>
#include "exult_constants.h"
#include "common_types.h"
#include "flags.h"
#include "rect.h"
#include "shapeid.h"
#include "tqueue.h"
#include "tiles.h"
#include "objlist.h"
#include "ignore_unused_variable_warning.h"

class Actor;
class Map_chunk;
class Container_game_object;
class Terrain_game_object;
class Egg_object;
class Barge_object;
class Game_window;
class Npc_actor;
class PathFinder;
class Rectangle;
class Schedule;
class Usecode_machine;
class Vga_file;
class ODataSource;
class Game_map;
class Object_client;

template<class T>
class T_Object_list;

typedef std::vector<Game_object *>   Game_object_vector;
typedef std::vector<Egg_object *>    Egg_vector;
typedef std::vector<Actor *>     Actor_vector;

/*
 *  A game object is a shape from shapes.vga along with info. about its
 *  position within its chunk.
 */
class Game_object : public ShapeID {
protected:
	static Game_object *editing;    // Obj. being edited by ExultStudio.
	Map_chunk *chunk;       // Chunk we're in, or NULL.
	unsigned char tx, ty;       // (X,Y) of shape within chunk, or if
	//   in a container, coords. within
	//   gump's rectangle.

	unsigned char lift;     // Raise by 4* this number.
	short quality;          // Some sort of game attribute.
	int get_cxi() const;
	int get_cyi() const;
	void remove_clients();
private:
	Game_object *next, *prev;   // ->next in chunk list or container.
public:
	typedef std::set<Game_object *> Game_object_set;
private:
	Game_object_set dependencies;   // Objects which must be painted before
	//   this can be rendered.
	Game_object_set dependors;  // Objects which must be painted after.
	static unsigned char rotate[8]; // For getting rotated frame #.
	std::vector <Object_client *> clients; // Notify when deleted.
public:
	uint32 render_seq;      // Render sequence #.
public:
	friend class T_Object_list<Game_object *>;
	friend class T_Object_iterator<Game_object *>;
	friend class T_Flat_object_iterator<Game_object *, Map_chunk *>;
	friend class T_Object_iterator_backwards < Game_object *,
			Map_chunk * >;
	friend class Map_chunk;
	Game_object(int shapenum, int framenum, unsigned int tilex,
	            unsigned int tiley, unsigned int lft = 0)
		: ShapeID(shapenum, framenum), chunk(0), tx(tilex), ty(tiley),
		  lift(lft), quality(0), render_seq(0)
	{  }
	// Copy constructor.
	Game_object(const Game_object &obj2)
		: ShapeID(obj2), chunk(obj2.chunk), tx(obj2.tx), ty(obj2.ty),
		  lift(obj2.lift), quality(obj2.quality), render_seq(0)
	{  }
	Game_object() : ShapeID(), chunk(0), render_seq(0)  // Create fake entry.
	{  }
	virtual ~Game_object()
	{  }
	int get_tx() const {    // Get tile (0-15) within chunk.
		return tx;
	}
	int get_ty() const {
		return ty;
	}
	int get_lift() const {
		return lift;
	}
	Tile_coord get_tile() const;    // Get location in abs. tiles.
	Tile_coord get_center_tile() const; // Get center location in abs. tiles.
	// Get missile start location in abs. tiles given dir.
	Tile_coord get_missile_tile(int dir) const;
	// Get distance to another object.
	int distance(Game_object *o2) const;
	// Get direction to another object.
	int distance(Tile_coord t2) const;
	// Get direction to another object.
	int get_direction(Game_object *o2) const;
	int get_direction(Tile_coord const &t2) const;
	int get_facing_direction(Game_object *o2) const;
	Map_chunk *get_chunk() const {  // Get chunk this is in.
		return chunk;
	}
	Game_map *get_map() const; // Map we're on.
	int get_map_num() const; // Get map number this is in.
	int get_cx() const;
	int get_cy() const;
	int get_quality() const {
		return quality;
	}
	void set_quality(int q) {
		quality = q;
	}
	int get_quantity() const;   // Like # of coins.
	int get_effective_obj_hp(int weapon_shape = 0) const;   // hitpoints for non-NPCs
	virtual int get_obj_hp() const; // hitpoints for non-NPCs
	virtual void set_obj_hp(int hp);
	int get_volume() const;     // Get space taken.
	// Add/remove to/from quantity.
	int modify_quantity(int delta, bool *del = 0);
	// Set shape coord. in chunk/gump.
	void set_shape_pos(unsigned int shapex, unsigned int shapey) {
		tx = shapex;
		ty = shapey;
	}
	void set_lift(int l) {
		lift = l;
	}
	Game_object *get_next() {
		return next;
	}
	Game_object *get_prev() {
		return prev;
	}
	// Compare for render order.
	static int compare(class Ordering_info &inf1, Game_object *obj2);
	int lt(Game_object &obj2);  // Is this less than another in pos.?
	void set_invalid() {    // Set to invalid position.
		chunk = 0;
	}
	bool is_pos_invalid() const {
		return chunk == 0;
	}
	bool inside_locked() const;
	void set_chunk(Map_chunk *c) {
		chunk = c;
	}
	// Get frame for desired direction.
	int get_dir_framenum(int dir, int frnum) const {
		return (frnum & 0xf) + rotate[dir];
	}
	// Get it using current dir.
	int get_dir_framenum(int frnum) const {
		return (frnum & 0xf) + (get_framenum() & (16 | 32));
	}
	// Get direction (NPC) is facing.
	int get_dir_facing() const;
	// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	void move(Tile_coord const &t, int newmap = -1) {
		move(t.tx, t.ty, t.tz, newmap);
	}
	bool add_client(Object_client *c);
	void remove_client(Object_client *c);
	void change_frame(int frnum);   // Change frame & set to repaint.
	// Swap positions.
	int swap_positions(Game_object *obj2);
	Game_object_set &get_dependencies() {
		return dependencies;
	}
	void clear_dependencies();  // Remove all dependencies.

	class Egg_cast_functor {
	public:
		Egg_object *operator()(Game_object *obj) const {
			return obj->as_egg();
		}
	};

	class Actor_cast_functor {
	public:
		Actor *operator()(Game_object *obj) const {
			return obj->as_actor();
		}
	};

	class Game_object_cast_functor {
	public:
		Game_object *operator()(Game_object *obj) const {
			return obj;
		}
	};

	template <typename VecType, typename Cast>
	static int find_nearby(VecType &vec, Tile_coord const &pos,
	                       int shapenum, int delta, int mask, int qual,
	                       int framenum, Cast const &obj_cast, bool exclude_okay_to_take = false);

	static int find_nearby_actors(Actor_vector &vec, Tile_coord const &pos,
	                              int shapenum, int delta, int mask = 8);
	static int find_nearby_eggs(Egg_vector &vec, Tile_coord const &pos,
	                            int shapenum, int delta, int qual = c_any_qual,
	                            int framenum = c_any_framenum);
	static int find_nearby(Game_object_vector &vec, Tile_coord const &pos,
	                       int shapenum, int delta, int mask, int qual = c_any_qual,
	                       int framenum = c_any_framenum, bool exclude_okay_to_take = false);
	int find_nearby_actors(Actor_vector &vec, int shapenum, int delta,
	                       int mask = 8) const;
	int find_nearby_eggs(Egg_vector &vec, int shapenum, int delta,
	                     int qual = c_any_qual, int frnum = c_any_framenum) const;
	int find_nearby(Game_object_vector &vec, int shapenum, int delta,
	                int mask, int qual = c_any_qual, int framenum = c_any_framenum) const;

	Game_object *find_closest(Game_object_vector &vec,
	                          int *shapenums, int num_shapes, int dist = 24);
	static Game_object *find_closest(Tile_coord const &pos,
	                                 int *shapenums, int num_shapes, int dist = 24);
	Game_object *find_closest(int *shapenums, int num_shapes,
	                          int dist = 24) {
		return find_closest(get_tile(), shapenums,
		                    num_shapes, dist);
	}
	Game_object *find_closest(int shapenum, int dist = 24) {
		return find_closest(&shapenum, 1, dist);
	}
	static Game_object *find_closest(Tile_coord const &pos, int shapenum,
	                                 int dist = 24) {
		return find_closest(pos, &shapenum, 1, dist);
	}
	Rectangle get_footprint();  // Get tile footprint.
	Block get_block();
	bool blocks(Tile_coord const &tile);    // Do we block a given tile?
	// Find object blocking given tile.
	static Game_object *find_blocking(Tile_coord tile);
	static Game_object *find_door(Tile_coord tile);
	int is_closed_door() const; // Checking for a closed door.
	Game_object *get_outermost();   // Get top 'owner' of this object.
	void say(const char *text);     // Put text up by item.
	void say(int msgnum);       // Show given text msg.
	void say(int from, int to); // Show random msg. from 'text.flx'.
	// Render.
	virtual void paint();
	void paint_outline(Pixel_colors pix);
	// Make this class abstract.
	virtual void paint_terrain() = 0;
	// Can this be clicked on?
	virtual int is_findable() {
		return 1;
	}
	// Run usecode function.
	virtual void activate(int event = 1);
	virtual bool edit();        // Edit in ExultStudio.
	// Saved from ExultStudio.
	static void update_from_studio(unsigned char *data, int datalen);
	virtual std::string get_name() const;
	// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual Container_game_object *get_owner() const {
		return 0;
	}
	virtual void set_owner(Container_game_object *o) {
		ignore_unused_variable_warning(o);
	}
	static int get_weight(int shnum, int quant = 1);
	virtual int get_weight();
	virtual int get_max_weight() const;   // Get max. weight allowed.
	virtual int is_dragable() const;// Can this be dragged?
	// Drop another onto this.
	virtual int drop(Game_object *obj);
	// Set/clear/get actor flag.
	virtual void set_flag(int flag) {
		ignore_unused_variable_warning(flag);
	}
	virtual void clear_flag(int flag) {
		ignore_unused_variable_warning(flag);
	}
	virtual int get_flag(int flag) const  {
		ignore_unused_variable_warning(flag);
		return 0;
	}
	virtual void set_flag_recursively(int flag) {
		ignore_unused_variable_warning(flag);
	}
	virtual int get_type_flag(int flag) const {
		ignore_unused_variable_warning(flag);
		return 0;
	}

	virtual Actor *as_actor() {
		return 0;
	}
	virtual Npc_actor *as_npc() {
		return 0;
	}
	virtual Barge_object *as_barge() {
		return 0;
	}
	virtual Terrain_game_object *as_terrain() {
		return 0;
	}
	virtual Container_game_object *as_container() {
		return 0;
	}
	virtual Egg_object *as_egg() {
		return 0;
	}
	virtual int is_egg() const { // An egg?
		return 0;
	}
	virtual void read_attributes(unsigned char *buf, int len) {
		ignore_unused_variable_warning(buf, len);
	}
	// Count contained objs.
	virtual int count_objects(int shapenum, int qual = c_any_qual,
	                          int framenum = c_any_framenum) {
		ignore_unused_variable_warning(shapenum, qual, framenum);
		return 0;
	}
	// Get contained objs.
	virtual int get_objects(Game_object_vector &vec, int shapenum, int qual,
	                        int framenum) {
		ignore_unused_variable_warning(vec, shapenum, qual, framenum);
		return 0;
	}
	// Add an object.
	virtual bool add(Game_object *obj, bool dont_check = false,
	                 bool combine = false, bool noset = false);
	// Add to NPC 'ready' spot.
	virtual int add_readied(Game_object *obj, int index,
	                        int dont_check = 0, int force_pos = 0, bool noset = false) {
		ignore_unused_variable_warning(index, force_pos);
		return add(obj, dont_check != 0, false, noset);
	}
	virtual int add_quantity(int delta, int shapenum, int qual = c_any_qual,
	                         int framenum = c_any_framenum, bool dontcreate = false, bool temporary = false) {
		ignore_unused_variable_warning(shapenum, qual, framenum, dontcreate, temporary);
		return delta;
	}
	virtual int create_quantity(int delta, int shapenum, int qual,
	                            int framenum, bool temporary = false) {
		ignore_unused_variable_warning(shapenum, qual, framenum, temporary);
		return delta;
	}
	virtual int remove_quantity(int delta, int shapenum, int qual,
	                            int framenum) {
		ignore_unused_variable_warning(shapenum, qual, framenum);
		return delta;
	}
	virtual Game_object *find_item(int shapenum, int qual, int framenum) {
		ignore_unused_variable_warning(shapenum, qual, framenum);
		return 0;
	}
	// Get coord. where this was placed.
	virtual Tile_coord get_original_tile_coord() const {
		return get_tile();
	}
	// Move out of the way.
	virtual int move_aside(Actor *for_actor, int dir) {
		ignore_unused_variable_warning(for_actor, dir);
		return 0;    // For now.
	}
	// Get frame if rotated clockwise.
	virtual int get_rotated_frame(int quads);
	// Step onto an (adjacent) tile.
	virtual int step(Tile_coord t, int frame, bool force = false) {
		ignore_unused_variable_warning(t, frame, force);
		return 0;
	}
	virtual int is_monster() {
		return 0;
	}
	virtual Game_object *find_weapon_ammo(int weapon, int needed = 1,
	                                      bool recursive = false) {
		ignore_unused_variable_warning(weapon, needed, recursive);
		return 0;
	}
	virtual int get_effective_range(const Weapon_info *winf = 0, int reach = -1);
	int get_weapon_ammo(int weapon, int family, int proj, bool ranged,
	                    Game_object **ammo = 0, bool recursive = false);
	void play_hit_sfx(int weapon, bool ranged);
	virtual bool try_to_hit(Game_object *attacker, int attval) {
		ignore_unused_variable_warning(attacker, attval);
		return true;
	}
	// Under attack.
	virtual Game_object *attacked(Game_object *attacker, int weapon_shape = -1,
	                              int ammo_shape = -1, bool explosion = false);
	// Hit-point algorithm:
	virtual int figure_hit_points(Game_object *attacker, int weapon_shape = -1,
	                              int ammo_shape = -1, bool explosion = false);
	virtual int apply_damage(Game_object *attacker, int str,
	                         int wpoints, int type, int bias = 0, int *exp = 0);
	virtual int reduce_health(int delta, int damage_type, Game_object *attacker = 0,
	                          int *exp = 0);
	// Write out to IREG file.
	virtual void write_ireg(ODataSource *out) {
		ignore_unused_variable_warning(out);
	}
	// Get size of IREG. Returns -1 if can't write to buffer
	virtual int get_ireg_size() {
		return 0;
	}
	// Write out IFIX, CHUNKS.
	virtual void write_ifix(ODataSource *ifix, bool v2) {
		ignore_unused_variable_warning(ifix, v2);
	}
	virtual void elements_read()    // Called when all member items read.
	{  }
	virtual int get_live_npc_num() const {
		return -1;
	}

	virtual void delete_contents() { }

	// Return's the object's usecode for the shape number
	virtual int get_usecode() const;
	// Default:  Can't set it.
	virtual bool set_usecode(int funid, const char *nm = 0);
};

/*
 *  Object from U7chunks.
 */
class Terrain_game_object : public Game_object {
	ShapeID prev_flat;
public:
	Terrain_game_object(int shapenum, int framenum, unsigned int tilex,
	                    unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft),
		  prev_flat(ShapeID(12, 0))
	{  }
	virtual ~Terrain_game_object() {  }
	virtual Terrain_game_object *as_terrain() {
		return this;
	}
	// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual void paint_terrain();
};

/*
 *  Object from an IFIXxx file.
 */
class Ifix_game_object : public Game_object {
public:
	Ifix_game_object(int shapenum, int framenum, unsigned int tilex,
	                 unsigned int tiley, unsigned int lft = 0)
		: Game_object(shapenum, framenum, tilex, tiley, lft)
	{  }
	virtual ~Ifix_game_object() {  }
	// Move to new abs. location.
	virtual void move(int newtx, int newty, int newlift, int newmap = -1);
	// Remove/delete this object.
	virtual void remove_this(int nodel = 0);
	virtual void paint_terrain() {  }
	virtual void write_ifix(ODataSource *ifix, bool v2);
};

#endif
