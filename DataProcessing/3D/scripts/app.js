/**
 * Atmospheric Layer Visualization Implementation (Matthew Arenivas)
 * 
 * This implementation visualizes three main atmospheric layers:
 * - Troposphere (0-10km): Blue, where most weather phenomena occur
 * - Stratosphere (10-20km): Cyan, contains the ozone layer
 * - Mesosphere (20-35km): Yellow, where meteors usually burn up
 * - (Calculations can we wrong) lol
 * 
 * Each layer is represented by a semi-transparent cylinder centered on the flight path.
 * 
 * Customization Options:
 * - Layer heights: Modify the 'height' values in atmosphereLayers array
 * - Colors: Adjust the 'color' values using Cesium.Color
 * - Cylinder size: Modify topRadius and bottomRadius in createFlight()
 * - Center position: Adjust the position calculation in createFlight()
 * 
 * Interactive Features:
 * - Toggle layer visibility using checkboxes
 * - Adjust layer opacity using sliders
 * - Click on flight points to see altitude and current layer
 */

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
let atmosphereLayers = [];

// Add data analysis helper functions (Matthew Arenivas)
function analyzeFlightData(data) {
  return {
    height: {
      min: Math.min(...data.map(d => parseFloat(d.height))),
      max: Math.max(...data.map(d => parseFloat(d.height)))
    },
    coordinates: {
      longitude: {
        min: Math.min(...data.map(d => parseFloat(d.longitude))),
        max: Math.max(...data.map(d => parseFloat(d.longitude)))
      },
      latitude: {
        min: Math.min(...data.map(d => parseFloat(d.latitude))),
        max: Math.max(...data.map(d => parseFloat(d.latitude)))
      }
    }
  };
}

// Update atmosphere layers to be dynamic (Matthew Arenivas)
function calculateAtmosphereLayers(flightStats) {
  const { min, max } = flightStats.height;
  const range = max - min;
  const layerCount = 3;
  const layerHeight = range / layerCount;

  return [
    {
      height: min + layerHeight,
      name: `Troposphere (${(min/1000).toFixed(1)}-${((min + layerHeight)/1000).toFixed(1)}km)`,
      color: Cesium.Color.BLUE.withAlpha(0.2)
    },
    {
      height: min + (layerHeight * 2),
      name: `Stratosphere (${((min + layerHeight)/1000).toFixed(1)}-${((min + layerHeight * 2)/1000).toFixed(1)}km)`,
      color: Cesium.Color.CYAN.withAlpha(0.15)
    },
    {
      height: max,
      name: `Mesosphere (${((min + layerHeight * 2)/1000).toFixed(1)}-${(max/1000).toFixed(1)}km)`,
      color: Cesium.Color.YELLOW.withAlpha(0.1)
    }
  ].map(layer => ({ ...layer, height: layer.height }));
}

// Update getCurrentAtmosphereLayer to use dynamic ranges (Matthew Arenivas)
function getCurrentAtmosphereLayer(height, flightStats) {
  const { min, max } = flightStats.height;
  const range = max - min;
  const layerHeight = range / 3;

  const heightInKm = height / 1000;
  const firstThreshold = (min + layerHeight) / 1000;
  const secondThreshold = (min + layerHeight * 2) / 1000;

  if (heightInKm < firstThreshold) return `Lower Layer (${(min/1000).toFixed(1)}-${firstThreshold.toFixed(1)}km)`;
  if (heightInKm < secondThreshold) return `Middle Layer (${firstThreshold.toFixed(1)}-${secondThreshold.toFixed(1)}km)`;
  return `Upper Layer (${secondThreshold.toFixed(1)}-${(max/1000).toFixed(1)}km)`;
}

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

// Update createFlight to use dynamic calculations (Matthew Arenivas)
function createFlight() {
  sampledPositionProperty = new Cesium.SampledPositionProperty();

  // Analyze flight data for dynamic ranges
  const flightStats = analyzeFlightData(flightData);
  atmosphereLayers = calculateAtmosphereLayers(flightStats);

  // Calculate center position for cylinders
  const centerLongitude = (flightStats.coordinates.longitude.min + flightStats.coordinates.longitude.max) / 2;
  const centerLatitude = (flightStats.coordinates.latitude.min + flightStats.coordinates.latitude.max) / 2;

  // Calculate appropriate cylinder radius based on coordinate spread
  const longitudeSpread = Math.abs(flightStats.coordinates.longitude.max - flightStats.coordinates.longitude.min);
  const latitudeSpread = Math.abs(flightStats.coordinates.latitude.max - flightStats.coordinates.latitude.min);
  const radiusInDegrees = Math.max(longitudeSpread, latitudeSpread);
  const cylinderRadius = radiusInDegrees * 111000 / 2; // Convert degrees to meters (roughly)

  // Add atmosphere layer cylinders
  atmosphereLayers.forEach((layer, index) => {
    viewer.entities.add({
      name: layer.name,
      cylinder: {
        length: layer.height,
        topRadius: cylinderRadius,
        bottomRadius: cylinderRadius,
        material: layer.color,
        outline: true,
        outlineColor: Cesium.Color.WHITE.withAlpha(0.3),
        slicePartitions: 4,
        shadows: Cesium.ShadowMode.ENABLED
      },
      label: {
        text: layer.name,
        font: '14pt sans-serif',
        style: Cesium.LabelStyle.FILL_AND_OUTLINE,
        outlineWidth: 2,
        verticalOrigin: Cesium.VerticalOrigin.BOTTOM,
        pixelOffset: new Cesium.Cartesian2(0, -10)
      },
      position: Cesium.Cartesian3.fromDegrees(
        centerLongitude,
        centerLatitude,
        layer.height / 2
      ),
    });
  });

  // Iterate through the flight data and add samples to the position property
  for (let i = 0; i < flightData.length; i++) {
    const dataPoint = flightData[i];
    
    // Calculate speed if we have a previous position (Matthew Arenivas)
    let speed = 0;
    if (i > 0) {
      const prevPoint = flightData[i - 1];
      const prevTime = new Date(prevPoint.timestamp).getTime();
      const currentTime = new Date(dataPoint.timestamp).getTime();
      const timeDiff = (currentTime - prevTime) / 1000; // Convert to seconds
      
      // Calculate distance between points
      const prevPos = Cesium.Cartesian3.fromDegrees(
        parseFloat(prevPoint.longitude),
        parseFloat(prevPoint.latitude),
        parseFloat(prevPoint.height)
      );
      const currentPos = Cesium.Cartesian3.fromDegrees(
        parseFloat(dataPoint.longitude),
        parseFloat(dataPoint.latitude),
        parseFloat(dataPoint.height)
      );
      const distance = Cesium.Cartesian3.distance(prevPos, currentPos);
      speed = (distance / timeDiff) * 3.6; // Convert to km/h
    }
  
    // Update the time and position for each data point
    const time = Cesium.JulianDate.fromIso8601(dataPoint.timestamp);
    const position = Cesium.Cartesian3.fromDegrees(
      parseFloat(dataPoint.longitude), 
      parseFloat(dataPoint.latitude), 
      parseFloat(dataPoint.height)
    );
    sampledPositionProperty.addSample(time, position);

    // Update description to use dynamic layer calculation
    let description = `
    <strong>Longitude:</strong> ${dataPoint.longitude}°</br>
    <strong>Latitude:</strong> ${dataPoint.latitude}°</br>
    <strong>Height:</strong> ${(dataPoint.height * 3.28084).toFixed(2)} ft</br>
    <strong>Speed:</strong> ${speed.toFixed(2)} km/h</br>
    <strong>Layer:</strong> ${getCurrentAtmosphereLayer(parseFloat(dataPoint.height), flightStats)}</br>
    <strong>Time:</strong> ${dataPoint.timestamp}</br>`;
  
    // Create a point entity for each data point with color based on speed (Matthew Arenivas)
    const pointColor = getColorBasedOnSpeed(speed);
    viewer.entities.add({
      name: "Flight Data Point",
      description: description,
      position: position,
      point: { 
        pixelSize: 10, 
        color: pointColor,
        outlineColor: Cesium.Color.WHITE,
        outlineWidth: 2
      }
    });
  }
}

// Helper function to get color based on speed (Matthew Arenivas)
function getColorBasedOnSpeed(speed) {
  if (speed === 0) return Cesium.Color.BLUE.withAlpha(0.8);
  if (speed < 50) return Cesium.Color.GREEN.withAlpha(0.8);
  if (speed < 100) return Cesium.Color.YELLOW.withAlpha(0.8);
  return Cesium.Color.RED.withAlpha(0.8);
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

// Add UI controls for layer visibility and opacity (Matthew Arenivas)
function createLayerControls() {
  const controlPanel = document.createElement('div');
  controlPanel.id = 'layerControls';
  controlPanel.style.position = 'absolute';
  controlPanel.style.top = '10px';
  controlPanel.style.left = '10px';
  controlPanel.style.backgroundColor = 'rgba(0, 0, 0, 0.7)';
  controlPanel.style.padding = '10px';
  controlPanel.style.color = 'white';
  controlPanel.style.borderRadius = '5px';

  atmosphereLayers.forEach((layer, index) => {
    const layerDiv = document.createElement('div');
    layerDiv.style.marginBottom = '10px';
    
    // Visibility toggle
    const checkbox = document.createElement('input');
    checkbox.type = 'checkbox';
    checkbox.checked = true;
    checkbox.onchange = (e) => {
      const entity = viewer.entities.values[index];
      entity.show = e.target.checked;
    };

    // Opacity slider
    const slider = document.createElement('input');
    slider.type = 'range';
    slider.min = '0';
    slider.max = '100';
    slider.value = '20';
    slider.onchange = (e) => {
      const entity = viewer.entities.values[index];
      const alpha = e.target.value / 100;
      entity.cylinder.material = layer.color.withAlpha(alpha);
    };

    layerDiv.innerHTML = `
      <label>${layer.name}</label><br/>
      <span>Visible: </span>
    `;
    layerDiv.appendChild(checkbox);
    layerDiv.innerHTML += '<br/><span>Opacity: </span>';
    layerDiv.appendChild(slider);
    
    controlPanel.appendChild(layerDiv);
  });

  document.body.appendChild(controlPanel);
}

// Add altitude graph visualization (Matthew Arenivas)
function createAltitudeGraph() {
  // Remove any existing graph
  const existingGraph = document.getElementById('altitudeGraph');
  if (existingGraph) {
    existingGraph.remove();
  }

  const graphContainer = document.createElement('div');
  graphContainer.id = 'altitudeGraph';
  graphContainer.style.position = 'absolute';
  graphContainer.style.bottom = '30px';
  graphContainer.style.right = '10px';
  graphContainer.style.width = '300px';
  graphContainer.style.height = '150px';
  graphContainer.style.backgroundColor = 'rgba(0, 0, 0, 0.8)'; // More opaque
  graphContainer.style.padding = '10px';
  graphContainer.style.borderRadius = '5px';
  graphContainer.style.zIndex = '9999'; // Very high z-index
  graphContainer.style.border = '1px solid white'; // Add border for visibility
  graphContainer.style.pointerEvents = 'none'; // Prevent blocking mouse events
  
  const title = document.createElement('div');
  title.style.color = 'white';
  title.style.marginBottom = '5px';
  title.style.fontWeight = 'bold';
  title.textContent = 'Flight Altitude Graph';
  graphContainer.appendChild(title);
  
  const canvas = document.createElement('canvas');
  canvas.width = 280;
  canvas.height = 130;
  canvas.style.display = 'block';
  graphContainer.appendChild(canvas);

  // Add to cesiumContainer instead of body
  const cesiumContainer = document.getElementById('cesiumContainer');
  if (cesiumContainer) {
    cesiumContainer.appendChild(graphContainer);
  } else {
    console.error('Could not find cesiumContainer');
    document.body.appendChild(graphContainer);
  }

  console.log('Graph container created:', graphContainer); // Debug log

  // Update graph on clock tick
  viewer.clock.onTick.addEventListener((clock) => {
    updateAltitudeGraph(canvas, clock);
  });
}

function updateAltitudeGraph(canvas, clock) {
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  
  const flightStats = analyzeFlightData(flightData);
  const { min, max } = flightStats.height;
  
  // Constants for graph layout
  const padding = {
    left: 50,    // Space for y-axis labels
    right: 10,
    top: 10,
    bottom: 30   // Space for x-axis labels
  };
  const graphWidth = canvas.width - padding.left - padding.right;
  const graphHeight = canvas.height - padding.top - padding.bottom;

  // Draw background
  ctx.fillStyle = 'rgba(0, 0, 0, 0.8)';
  ctx.fillRect(0, 0, canvas.width, canvas.height);
  
  // Draw grid lines and labels
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
  ctx.fillStyle = 'white';
  ctx.font = '10px Arial';
  ctx.textAlign = 'right';
  
  // Horizontal grid lines (altitude)
  const altitudeStep = Math.ceil((max - min) / 5 / 1000) * 1000; // Round to nearest 1000m
  for (let alt = 0; alt <= max; alt += altitudeStep) {
    const y = padding.top + graphHeight - (alt / max) * graphHeight;
    
    // Grid line
    ctx.beginPath();
    ctx.moveTo(padding.left, y);
    ctx.lineTo(canvas.width - padding.right, y);
    ctx.stroke();
    
    // Label (in km)
    ctx.fillText((alt / 1000).toFixed(1) + 'km', padding.left - 5, y + 4);
  }

  // Draw atmospheric layers
  const layers = calculateAtmosphereLayers(flightStats);
  layers.forEach((layer, i) => {
    const y = padding.top + graphHeight - (layer.height / max) * graphHeight;
    ctx.fillStyle = layer.color.toCssColorString();
    ctx.fillRect(
      padding.left,
      y,
      graphWidth,
      (layer.height / max) * graphHeight
    );
  });

  // Plot altitude line with gradient
  const gradient = ctx.createLinearGradient(0, padding.top, 0, padding.top + graphHeight);
  gradient.addColorStop(0, 'rgba(255, 100, 100, 1)');  // Red at high altitude
  gradient.addColorStop(1, 'rgba(100, 255, 100, 1)');  // Green at low altitude

  ctx.beginPath();
  ctx.strokeStyle = gradient;
  ctx.lineWidth = 2;
  
  flightData.forEach((point, i) => {
    const x = padding.left + (i / (flightData.length - 1)) * graphWidth;
    const y = padding.top + graphHeight - (point.height / max) * graphHeight;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.stroke();

  // Draw current position marker
  const currentTime = Cesium.JulianDate.toDate(clock.currentTime);
  const currentIndex = flightData.findIndex(point => 
    new Date(point.timestamp) > currentTime
  );
  
  if (currentIndex > 0) {
    const x = padding.left + (currentIndex / (flightData.length - 1)) * graphWidth;
    const y = padding.top + graphHeight - (flightData[currentIndex].height / max) * graphHeight;
    
    // Draw vertical time indicator line
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.5)';
    ctx.setLineDash([5, 5]);
    ctx.beginPath();
    ctx.moveTo(x, padding.top);
    ctx.lineTo(x, padding.top + graphHeight);
    ctx.stroke();
    ctx.setLineDash([]);

    // Draw current position marker
    ctx.fillStyle = 'white';
    ctx.strokeStyle = 'black';
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.arc(x, y, 4, 0, Math.PI * 2);
    ctx.fill();
    ctx.stroke();

    // Show current altitude
    const currentAlt = (flightData[currentIndex].height / 1000).toFixed(1);
    ctx.fillStyle = 'white';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'left';
    ctx.fillText(`${currentAlt}km`, x + 8, y - 8);
  }

  // Add border
  ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
  ctx.lineWidth = 1;
  ctx.strokeRect(padding.left, padding.top, graphWidth, graphHeight);
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

    // Create layer controls
    createLayerControls();

    // Create altitude graph
    createAltitudeGraph();


  } catch (error) {
    console.error('Error loading JSON data:', error);
  }
}

// Call main function to initialize the viewer
init();

/* Oringinal Code can be found below in case this is trash

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
*/