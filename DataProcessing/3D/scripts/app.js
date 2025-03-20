// Provide access token for Cesium Ion
Cesium.Ion.defaultAccessToken = 'your_access_token';

// Initialize the Cesium viewer with customized settings
const viewer = new Cesium.Viewer('cesiumContainer', {
  terrain: Cesium.Terrain.fromWorldTerrain(),
});
viewer.scene.globe.enableLighting = true;

// Declare global variables
let flightData = [];
let start, stop, timeStepInSeconds;
let sampledPositionProperty;

// Initialize the viewer's clock 
function setClockFromData() {
  const times = flightData.map(dp => Cesium.JulianDate.fromIso8601(dp.timestamp));
  
  // Determine the earliest and latest times in data (start & stop)
  start = times.reduce((min, t) => Cesium.JulianDate.lessThan(t, min) ? t : min, times[0]);
  stop = times.reduce((max, t) => Cesium.JulianDate.greaterThan(t, max) ? t : max, times[0]);
  viewer.clock.startTime = Cesium.JulianDate.clone(start);
  viewer.clock.stopTime = Cesium.JulianDate.clone(stop);
  viewer.clock.currentTime = Cesium.JulianDate.clone(start);

  viewer.timeline.zoomTo(start, stop);
  viewer.clock.shouldAnimate = true;
}

// Create flight entities and sample the positions
function createFlight() {
  sampledPositionProperty = new Cesium.SampledPositionProperty();

  // Iterate through the flight data and add samples to the position property
  for (let i = 0; i < flightData.length; i++) {
    const dataPoint = flightData[i];
  
    // Update the time and position for each data point
    const time = Cesium.JulianDate.fromIso8601(dataPoint.timestamp);
    const position = Cesium.Cartesian3.fromDegrees(
      parseFloat(dataPoint.longitude), 
      parseFloat(dataPoint.latitude), 
      parseFloat(dataPoint.height)
    );
    sampledPositionProperty.addSample(time, position);

    // Add a description to each data point
    let description = `
    <strong>Longitude:</strong> ${dataPoint.longitude}</br>
    <strong>Latitude:</strong> ${dataPoint.latitude}</br>
    <strong>Height:</strong> ${(dataPoint.height * 3.28084).toFixed(2)} ft</br>
    <strong>Time:</strong> ${dataPoint.timestamp}</br>`;
  
  // Create a point entity for each data point
  viewer.entities.add({
    name: "Flight Data Point",
    description: description,
    position: position,
    point: { pixelSize: 10, color: Cesium.Color.RED }
  });
  
  }
}

// Create the main flight entity that uses the sampled positions
function createEntity() {
  const payloadEntity = viewer.entities.add({
    availability: new Cesium.TimeIntervalCollection([ 
      new Cesium.TimeInterval({ start: start, stop: stop }) 
    ]),
    position: sampledPositionProperty,
    name: 'ASU ASCEND Payload',
    billboard: {
      image: '../images/payload.png',
      width: 150,
      height: 150,
      horizontalOrigin: Cesium.HorizontalOrigin.CENTER,
      verticalOrigin: Cesium.VerticalOrigin.BOTTOM
    },
    path: new Cesium.PathGraphics({ width: 2 })
  });
  // Make the camera track this moving entity.
  viewer.trackedEntity = payloadEntity;
}

// Load JSON data from the aprs-data-fall-2024.json file and initialize everything
async function init() {
  try {
    const response = await fetch('../data/aprs-data-fall-2024.json');
    flightData = await response.json();
    console.log(flightData);
    
    // Set the clock
    setClockFromData();

    // Create flight entities and sample the positions
    createFlight();

    // Create the main flight entity that uses the sampled positions
    createEntity();

  } catch (error) {
    console.error('Error loading JSON data:', error);
  }
}

// Call main function to initialize the viewer
init();
