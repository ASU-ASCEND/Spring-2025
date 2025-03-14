// Provide access token for Cesium Ion
Cesium.Ion.defaultAccessToken = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJmNzRmZDQ5MS0zODY1LTRjYjEtOGI3Ny0wZGMwNWQ1MjVhOGMiLCJpZCI6MjI1Mjg3LCJpYXQiOjE3MjA0NTM1NzV9.k5jpBA4KmrErsokf_kxNKNcYbE8tNwCavyzJNRQOFeQ';

// Initialize the Cesium viewer with customized settings
const viewer = new Cesium.Viewer('cesiumContainer', {
  terrain: Cesium.Terrain.fromWorldTerrain(),
});
viewer.scene.globe.enableLighting = true;

// Declare these variables in a scope accessible to all functions
let flightData = [];
let start, stop, timeStepInSeconds;
let sampledPositionProperty;

// Initialize the viewer's clock using flightData length
function setClock() {
  timeStepInSeconds = 30;
  const totalSeconds = timeStepInSeconds * (flightData.length - 1);
  start = Cesium.JulianDate.fromIso8601("2020-03-09T23:10:00Z");
  stop = Cesium.JulianDate.addSeconds(start, totalSeconds, new Cesium.JulianDate());
  
  viewer.clock.startTime = Cesium.JulianDate.clone(start);
  viewer.clock.stopTime = Cesium.JulianDate.clone(stop);
  viewer.clock.currentTime = Cesium.JulianDate.clone(start);
  viewer.timeline.zoomTo(start, stop);
  viewer.clock.shouldAnimate = true;
}

// Create flight entities and sample the positions
function createFlight() {
  // Initialize the sampled position property
  sampledPositionProperty = new Cesium.SampledPositionProperty();

  for (let i = 0; i < flightData.length; i++) {
    const dataPoint = flightData[i];
  
    // Calculate time for each sample using global start and timeStepInSeconds
    const time = Cesium.JulianDate.addSeconds(start, i * timeStepInSeconds, new Cesium.JulianDate());
    const position = Cesium.Cartesian3.fromDegrees(dataPoint.longitude, dataPoint.latitude, dataPoint.height);
    
    // Add the sample to the position property
    sampledPositionProperty.addSample(time, position);
    
    // Optionally, add an entity for each point (if needed for visualization)
    viewer.entities.add({
      description: `Location: (${dataPoint.longitude}, ${dataPoint.latitude}, ${dataPoint.height})`,
      position: position,
      point: { pixelSize: 10, color: Cesium.Color.RED }
    });
  }
}

// Create the main flight entity that uses the sampled positions
function createEntity() {
  const airplaneEntity = viewer.entities.add({
    availability: new Cesium.TimeIntervalCollection([ 
      new Cesium.TimeInterval({ start: start, stop: stop }) 
    ]),
    position: sampledPositionProperty,
    billboard: {
      image: '../images/payload.png',
      width: 150,
      height: 150,
      horizontalOrigin: Cesium.HorizontalOrigin.CENTER,
      verticalOrigin: Cesium.VerticalOrigin.BOTTOM
    },
    path: new Cesium.PathGraphics({width: 2})
  });
  // Make the camera track this moving entity.
  viewer.trackedEntity = airplaneEntity;
}

// Load JSON data from a aprs-data-fall-2024.json file and initialize everything
async function init() {
  try {
    const response = await fetch('../data/aprs-data-fall-2024.json');
    flightData = await response.json();
    console.log(flightData);
    
    // Set up the clock based on flightData
    setClock();

    // Create flight samples and individual entities
    createFlight();

    // Create the main flight entity to animate along the path
    createEntity();

  } catch (error) {
    console.error('Error loading JSON data:', error);
  }
}

init();
