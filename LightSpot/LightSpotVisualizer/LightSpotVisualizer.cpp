﻿#include <memory>
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <SFML/Graphics.hpp>

#include "../environment/LightSpotEnvironment.h"

using namespace std;
using namespace boost::interprocess;

class EnvironmentState
{
	unique_ptr<shared_memory_object> shm;
	unique_ptr<mapped_region>        region;
public:
	volatile pair<float, float> *pprr_CameraCenter;
	volatile pair<float, float> *pprr_SpotCenter = NULL;
	EnvironmentState()
	{
		do {
			try {
				//Create a shared memory object.
				shm.reset(new shared_memory_object(open_only, ENVIRONMENT_STATE_SHARED_MEMORY_NAME, read_only));

				//Set size
		//		shm->truncate(sizeof(pair<pair<float, float>, pair<float, float> >));

				//Map the whole shared memory in this process
				region.reset(new mapped_region(*shm, read_only));
				pprr_CameraCenter = (volatile pair<float, float> *)region->get_address();
				pprr_SpotCenter = &((volatile pair<pair<float, float>, pair<float, float> > *)pprr_CameraCenter)->second;
			} catch (...) {
				boost::this_thread::sleep(boost::posix_time::seconds(1));
			}
		} while (!pprr_SpotCenter);
	}
	~EnvironmentState() {}

	// NEEDED?

	double dDistance() const
	{
		return sqrt((pprr_CameraCenter->first - pprr_SpotCenter->first) * (pprr_CameraCenter->first - pprr_SpotCenter->first) + (pprr_CameraCenter->second - pprr_SpotCenter->second) * (pprr_CameraCenter->second - pprr_SpotCenter->second));
	}

};

inline float rPixel(float rPhysical) {return (rPhysical + 1.F) * EXACT_RASTER_SIZE;}

int main()
{
	cout << "Waiting for LightSpot data availability...\n";
	EnvironmentState es;
	cout << "LightSpot data are obtained\n";
	// Create the main window
	sf::RenderWindow window(sf::VideoMode(EXACT_RASTER_SIZE * 2, EXACT_RASTER_SIZE * 2), "LightSpot", sf::Style::Close);
	sf::CircleShape circle;
	circle.setRadius(SPOT_HALFSIZE_PIXEL);
	circle.setFillColor(sf::Color::White);
	circle.setOrigin(SPOT_HALFSIZE_PIXEL, SPOT_HALFSIZE_PIXEL);

	// Start the game loop
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			// Close window: exit
			if (event.type == sf::Event::Closed)
				window.close();
		}

		// Clear screen
		window.clear();
		circle.setPosition(rPixel(es.pprr_SpotCenter->first), rPixel(es.pprr_SpotCenter->second));
		window.draw(circle);

		// Update the window
		window.display();
	}
	return EXIT_SUCCESS;
}
