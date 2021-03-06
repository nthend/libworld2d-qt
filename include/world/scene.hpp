#pragma once

#include <QGraphicsScene>
#include <QTimer>

#include <map>
#include <cmath>
#include <string>
#include <functional>

#include <world/world.hpp>

#include "item.hpp"

class Scene : public QGraphicsScene {
public:
	std::map<int, Item*> items;
	World &world;
	
	bool done = false;
	
	Scene(World &w) : QGraphicsScene(),
		world(w) 
	{
		setBackgroundBrush(QColor("#222222"));
		setSceneRect(-world.size.x(), -world.size.y(), 2*world.size.x(), 2*world.size.y());
		addRect(sceneRect(), Qt::NoPen, QBrush(QColor("#000000")));
	}

	virtual ~Scene() {
		for(auto &p : items) {
			delete p.second;
		}
	}
	
	virtual Item *instance(const Entity *e) const {
		Item *item = new Item();
		item->sync(e);
		return item;
	}
	
	void sync_func() {
		sync();
		// update();
		if(!done) {
			QTimer::singleShot(40, [this](){sync_func();});
		}
	}
	
	void start() {
		done = false;
		sync_func();
	}
	
	void stop() {
		done = true;
	}
	
	void sync() {
		for(auto &p : items) {
			p.second->exist = false;
		}
		
		world.access.lock();
		{
			for(const auto &ep : world.entities) {
				int id = ep.first;
				const Entity *e = ep.second;
				Item *it = nullptr;
				auto ii = items.find(id);
				if(ii == items.end()) {
					it = instance(e);
					it->setZValue(id);
					ii = items.insert(std::pair<int, Item*>(id, it)).first;
					addItem(it);
				} else {
					it = ii->second;
					it->sync(e);
				}
				it->exist = true;
			}
		}
		world.access.unlock();
		
		for(auto ii = items.begin(); ii != items.end();) {
			if(ii->second->exist) {
				++ii;
			} else {
				auto iv = ii->second;
				removeItem(iv);
				items.erase(ii++);
				delete iv;
			}
		}
	}
};
