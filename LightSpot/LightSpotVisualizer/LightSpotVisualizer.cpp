#include <memory>
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#ifndef FOR_LINUX
#include <SFML/Graphics.hpp>
#endif

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
	EnvironmentState(bool bDestroyOldState)
	{
		if (!bDestroyOldState)
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
		else {
			string strSharedMemoryName = ENVIRONMENT_STATE_SHARED_MEMORY_NAME;
			bool bExists = true;
			do {
				try {
					//Create a shared memory object.
					shm.reset(new shared_memory_object(open_only, strSharedMemoryName.c_str(), read_only));
					shared_memory_object::remove(strSharedMemoryName.c_str());
					++strSharedMemoryName.front();
				}
				catch (...) {
					bExists = false;
				}
			} while (bExists);
		}
	}
	~EnvironmentState() {}

	// NEEDED?

	double dDistance() const
	{
		return sqrt((pprr_CameraCenter->first - pprr_SpotCenter->first) * (pprr_CameraCenter->first - pprr_SpotCenter->first) + (pprr_CameraCenter->second - pprr_SpotCenter->second) * (pprr_CameraCenter->second - pprr_SpotCenter->second));
	}

};

inline float rPixelX(float rPhysical) {return (rPhysical + 1.F) * EXACT_RASTER_SIZE;}
inline float rPixelY(float rPhysical) {return (1.F - rPhysical) * EXACT_RASTER_SIZE;}

int main(int ARGC, char *ARGV[])
{
#ifndef FOR_LINUX
	bool bDestroyOldState = ARGC == 2;
	cout << "Waiting for LightSpot data availability...\n";
	EnvironmentState es(bDestroyOldState);
	if (bDestroyOldState) {
		cout << "All resident LightSpot data are destroyed\n";
		exit(0);
	}
	cout << "LightSpot data are obtained\n";
	// Create the main window
	sf::RenderWindow window(sf::VideoMode(EXACT_RASTER_SIZE * 2, EXACT_RASTER_SIZE * 2), "LightSpot", sf::Style::Close);
	sf::CircleShape circle;
	circle.setRadius(SPOT_HALFSIZE_PIXEL);
	circle.setFillColor(sf::Color::White);
	circle.setOrigin(SPOT_HALFSIZE_PIXEL, SPOT_HALFSIZE_PIXEL);
	sf::RectangleShape rectangle;
	rectangle.setSize(sf::Vector2f(EXACT_RASTER_SIZE, EXACT_RASTER_SIZE));
	rectangle.setOutlineColor(sf::Color::White);
	rectangle.setOutlineThickness(1);
	rectangle.setOrigin(EXACT_RASTER_SIZE / 2, EXACT_RASTER_SIZE / 2);
	rectangle.setFillColor(sf::Color::Transparent);

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
		circle.setPosition(rPixelX(es.pprr_SpotCenter->first), rPixelY(es.pprr_SpotCenter->second));
		window.draw(circle);
		rectangle.setPosition(rPixelX(es.pprr_CameraCenter->first), rPixelY(es.pprr_CameraCenter->second));
		window.draw(rectangle);

		// Update the window
		window.display();
	}
	return EXIT_SUCCESS;
#else
	EnvironmentState es(true);
	cout << "All resident LightSpot data are destroyed\n";
	return 0;
#endif
}
