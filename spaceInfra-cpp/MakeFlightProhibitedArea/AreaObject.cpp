#include "AreaObject.h"

#include "GeoPolygon.h"

AreaObject::AreaObject() {
	Config& config = Config::getInstance();
	logger = spdlog::get(config.getLoggerName());
}

void AreaObject::addPolygon(GeoPolygon* pPolygon) {
	polygonContainer.addObjects(1, pPolygon);
	pPolygon->setPolygonId(polygonContainer.getObjectCount() - 1);

}