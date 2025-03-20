// Provide access token for Cesium Ion
Cesium.Ion.defaultAccessToken = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJmNzRmZDQ5MS0zODY1LTRjYjEtOGI3Ny0wZGMwNWQ1MjVhOGMiLCJpZCI6MjI1Mjg3LCJpYXQiOjE3MjA0NTM1NzV9.k5jpBA4KmrErsokf_kxNKNcYbE8tNwCavyzJNRQOFeQ';

// Initialize the Cesium viewer with customized settings
const viewer = new Cesium.Viewer('cesiumContainer', {
  terrain: Cesium.Terrain.fromWorldTerrain(),
});
viewer.scene.globe.enableLighting = true;

// Configure scene to reduce render artifacts with transparent objects
viewer.scene.globe.depthTestAgainstTerrain = true;
viewer.scene.logarithmicDepthBuffer = false;
viewer.scene.postProcessStages.fxaa.enabled = true;
viewer.scene.highDynamicRange = false;

// Declare global variables
let flightData = [];
let start, stop, timeStepInSeconds;
let sampledPositionProperty;
let atmosphereLayers = [];

/**
 * @brief Min and max data analysis helper functions  
 * 
 * Calculates and returns the min and max height, longitude, and latitude 
 * from the flight data.
 * 
 * @param {*} data 
 * @returns Min and max height, longitude, and latitude
 */
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

// Update atmosphere layers to be static 
/**
 * @brief Calculate atmosphere layers based on flight statistics
 * 
 * Calculate atmospheric layers including the Troposphere, Tropopause, 
 * Lower Stratosphere, Ozone Layer, and Upper Stratosphere based on flight 
 * statistics.
 * 
 * @param {*} flightStats 
 * @returns 
 */
function calculateAtmosphereLayers(flightStats) {
  const { min, max } = flightStats.height;  // These are in kilometers
  const layers = [
    {
      minHeight: 0,
      maxHeight: 11,  // Troposphere: 0-11km
      name: 'Troposphere (0-36,000ft)',
      color: new Cesium.Color.fromCssColorString("#95CADB"), // Blue
      baseAlpha: 0.2
    },
    {
      minHeight: 11,
      maxHeight: 12,  // Tropopause: 11-12km
      name: 'Tropopause (36,000-39,000ft)',
      color: new Cesium.Color.fromCssColorString("#588BAE"), // Red
      baseAlpha: 0.25
    },
    {
      minHeight: 12,
      maxHeight: 20,  // Lower Stratosphere: 12-20km
      name: 'Lower Stratosphere (39,000-65,600ft)',
      color: new Cesium.Color.fromCssColorString("#008ECC"), // Light Green
      baseAlpha: 0.15
    },
    {
      minHeight: 20,
      maxHeight: 30,  // Ozone Layer: 20-30km
      name: 'Ozone Layer (65,600-98,400ft)',
      color: new Cesium.Color.fromCssColorString("#1D2951"), // Magenta
      baseAlpha: 0.2
    },
    {
      minHeight: 30,
      maxHeight: 50,  // Upper Stratosphere: 30-50km
      name: 'Upper Stratosphere (98,400-164,000ft)',
      color: new Cesium.Color.fromCssColorString("#0147AB"), // Light Green
      baseAlpha: 0.15
    }
  ];

  // Return all layers instead of filtering
  return layers;
}

// Update getCurrentAtmosphereLayer to include only lower layers  
function getCurrentAtmosphereLayer(height, flightStats) {
  const heightInKm = height;  // Height is already in kilometers
  
  if (heightInKm < 11) return 'Troposphere (0-36,000ft)';
  if (heightInKm < 12) return 'Tropopause (36,000-39,000ft)';
  if (heightInKm < 20) return 'Lower Stratosphere (39,000-65,600ft)';
  if (heightInKm < 30) return 'Ozone Layer (65,600-98,400ft)';
  return 'Upper Stratosphere (98,400-164,000ft)';
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

// Update createFlight to improve layer labeling  
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
  const cylinderRadius = radiusInDegrees * 111000; // Increased radius for better visibility

  // Add atmosphere layer cylinders with correct heights and positions
  atmosphereLayers.forEach((layer, index) => {
    const layerHeight = layer.maxHeight - layer.minHeight;
    const layerEntity = viewer.entities.add({
      name: layer.name,
      show: true,
      cylinder: {
        length: layerHeight * 1000,
        topRadius: cylinderRadius,
        bottomRadius: cylinderRadius,
        material: new Cesium.ColorMaterialProperty(
          new Cesium.Color(
            layer.color.red,
            layer.color.green, 
            layer.color.blue, 
            layer.baseAlpha
          )
        ),
        outline: true,
        outlineColor: Cesium.Color.WHITE.withAlpha(0.3),
        slicePartitions: 4,
        shadows: Cesium.ShadowMode.ENABLED
      },
      position: Cesium.Cartesian3.fromDegrees(
        centerLongitude,
        centerLatitude,
        (layer.minHeight + (layerHeight / 2)) * 1000
      ),
    });

    // Store the layer entity in global array for opacity control
    if (!window.layerEntities) window.layerEntities = [];
    window.layerEntities[index] = layerEntity;
  });

  // Iterate through the flight data and add samples to the position property
  for (let i = 0; i < flightData.length; i++) {
    const dataPoint = flightData[i];
    
    // Calculate speed if we have a previous position
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

    // Update description to use correct layer calculation
    let description = `
    <strong>Longitude:</strong> ${dataPoint.longitude}°</br>
    <strong>Latitude:</strong> ${dataPoint.latitude}°</br>
    <strong>Height:</strong> ${(dataPoint.height * 3.28084).toFixed(2)} ft</br>
    <strong>Speed:</strong> ${speed.toFixed(2)} km/h</br>
    <strong>Layer:</strong> ${getCurrentAtmosphereLayer(parseFloat(dataPoint.height), flightStats)}</br>
    <strong>Time:</strong> ${dataPoint.timestamp}</br>`;
  
    // Create a point entity for each data point with color based on speed
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

// Helper function to get color based on speed  
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

/** 
 * @brief Create layer controls for opacity and visibility
 * 
*/
function createLayerControls() {
  const controlPanel = document.createElement('div');
  controlPanel.id = 'layerControls';
  controlPanel.style.position = 'absolute';
  controlPanel.style.top = '10px';
  controlPanel.style.left = '10px';
  controlPanel.style.backgroundColor = 'rgba(13, 37, 53, 0.85)';
  controlPanel.style.padding = '15px';
  controlPanel.style.color = 'white';
  controlPanel.style.borderRadius = '8px';
  controlPanel.style.border = '2px solid rgba(0, 195, 255, 0.5)';
  controlPanel.style.boxShadow = '0 0 15px rgba(0, 195, 255, 0.3)';
  controlPanel.style.fontFamily = "'Rajdhani', sans-serif";

  // Add master visibility toggle at the top
  const masterToggleDiv = document.createElement('div');
  masterToggleDiv.style.marginBottom = '15px';
  masterToggleDiv.style.paddingBottom = '10px';
  masterToggleDiv.style.borderBottom = '1px solid rgba(0, 195, 255, 0.3)';

  const masterTitle = document.createElement('div');
  masterTitle.innerHTML = '<span style="color: rgb(0, 195, 255); text-transform: uppercase; letter-spacing: 1px; font-size: 14px;">Atmospheric Layers</span>';
  masterToggleDiv.appendChild(masterTitle);

  const masterCheckbox = document.createElement('input');
  masterCheckbox.type = 'checkbox';
  masterCheckbox.checked = true;
  masterCheckbox.style.accentColor = 'rgb(0, 195, 255)';
  masterCheckbox.onchange = (e) => {
    if (window.layerEntities) {
      window.layerEntities.forEach((entity) => {
        if (entity) entity.show = e.target.checked;
      });
    }
  };

  const masterLabel = document.createElement('label');
  masterLabel.style.display = 'flex';
  masterLabel.style.alignItems = 'center';
  masterLabel.style.gap = '8px';
  masterLabel.style.marginTop = '5px';
  masterLabel.style.color = 'rgb(0, 195, 255)';
  masterLabel.style.fontSize = '12px';
  masterLabel.style.letterSpacing = '0.5px';
  masterLabel.appendChild(masterCheckbox);
  masterLabel.appendChild(document.createTextNode('SHOW ALL LAYERS'));
  
  masterToggleDiv.appendChild(masterLabel);
  controlPanel.appendChild(masterToggleDiv);

  atmosphereLayers.forEach((layer, index) => {
    const layerDiv = document.createElement('div');
    layerDiv.style.marginBottom = '15px';
    layerDiv.style.padding = '8px';
    layerDiv.style.backgroundColor = 'rgba(0, 195, 255, 0.1)';
    layerDiv.style.borderRadius = '4px';
    layerDiv.style.border = '1px solid rgba(0, 195, 255, 0.2)';
    
    const layerName = document.createElement('div');
    layerName.style.color = layer.color.withAlpha(1.0).toCssColorString();
    layerName.style.fontSize = '12px';
    layerName.style.marginBottom = '5px';
    layerName.style.letterSpacing = '0.5px';
    layerName.textContent = layer.name;
    layerDiv.appendChild(layerName);
    
    const sliderContainer = document.createElement('div');
    sliderContainer.style.display = 'flex';
    sliderContainer.style.alignItems = 'center';
    sliderContainer.style.gap = '8px';
    
    const opacityLabel = document.createElement('span');
    opacityLabel.textContent = 'OPACITY';
    opacityLabel.style.color = 'rgba(255, 255, 255, 0.7)';
    opacityLabel.style.fontSize = '10px';
    opacityLabel.style.letterSpacing = '0.5px';
    
    const slider = document.createElement('input');
    slider.type = 'range';
    slider.min = '0';
    slider.max = '100';
    slider.value = '20';
    slider.style.flex = '1';
    slider.style.accentColor = 'rgb(0, 195, 255)';
    slider.onchange = (e) => {
      if (window.layerEntities && window.layerEntities[index]) {
        const alpha = e.target.value / 100;
        
        // Special case: if alpha is 0, just hide the entity
        if (alpha <= 0.01) {
          window.layerEntities[index].show = false;
          return;
        }
        
        // Show the entity (in case it was hidden)
        window.layerEntities[index].show = true;
        
        // Get the current entity
        const entity = window.layerEntities[index];
        
        // Create a stripe material with controlled even/odd colors for better transparency
        const stripesMaterial = new Cesium.StripeMaterialProperty({
          evenColor: new Cesium.Color(
            layer.color.red,
            layer.color.green,
            layer.color.blue,
            alpha * 0.6  // Even stripes are slightly more transparent
          ),
          oddColor: new Cesium.Color(
            layer.color.red,
            layer.color.green,
            layer.color.blue,
            alpha  // Odd stripes use full alpha
          ),
          repeat: Math.max(2, Math.floor(16 * alpha)) // More stripes at higher opacity
        });
        
        // Apply the material to the cylinder
        entity.cylinder.material = stripesMaterial;
      }
    };

    sliderContainer.appendChild(opacityLabel);
    sliderContainer.appendChild(slider);
    layerDiv.appendChild(sliderContainer);
    
    controlPanel.appendChild(layerDiv);
  });

  document.body.appendChild(controlPanel);
}

/**
 * @brief Create, style, and position altitude graph visualization
 * 
 */
function createAltitudeGraph() {
  // Remove any existing graph
  const existingGraph = document.getElementById('altitudeGraph');
  if (existingGraph) {
    existingGraph.remove();
  }

  const graphContainer = document.createElement('div');
  graphContainer.id = 'altitudeGraph';
  graphContainer.style.position = 'absolute';
  graphContainer.style.top = '480px';  // Moved lower from 420px
  graphContainer.style.left = '10px';
  graphContainer.style.width = '350px';
  graphContainer.style.height = '200px';
  graphContainer.style.backgroundColor = 'rgba(13, 37, 53, 0.85)';
  graphContainer.style.padding = '15px';
  graphContainer.style.borderRadius = '8px';
  graphContainer.style.zIndex = '9999';
  graphContainer.style.border = '2px solid rgba(0, 195, 255, 0.5)';
  graphContainer.style.boxShadow = '0 0 15px rgba(0, 195, 255, 0.3)';
  graphContainer.style.pointerEvents = 'auto';
  graphContainer.style.fontFamily = "'Rajdhani', 'Orbitron', sans-serif";
  
  const titleContainer = document.createElement('div');
  titleContainer.style.display = 'flex';
  titleContainer.style.alignItems = 'center';
  titleContainer.style.marginBottom = '5px';
  
  const title = document.createElement('div');
  title.style.color = 'rgb(0, 195, 255)';
  title.style.fontWeight = 'bold';
  title.style.textTransform = 'uppercase';
  title.style.letterSpacing = '1px';
  title.style.fontSize = '14px';
  title.textContent = 'Flight Altitude Analysis';
  titleContainer.appendChild(title);
  graphContainer.appendChild(titleContainer);
  
  // Create toggle button outside graph container
  const toggleButton = document.createElement('button');
  toggleButton.textContent = 'HIDE GRAPH';
  toggleButton.style.position = 'absolute';
  toggleButton.style.top = '445px';  // Moved lower from 385px
  toggleButton.style.left = '10px';
  toggleButton.style.padding = '5px 12px';
  toggleButton.style.backgroundColor = 'rgba(13, 37, 53, 0.9)';
  toggleButton.style.border = '2px solid rgba(0, 195, 255, 0.5)';
  toggleButton.style.borderRadius = '4px';
  toggleButton.style.color = 'rgb(0, 195, 255)';
  toggleButton.style.cursor = 'pointer';
  toggleButton.style.zIndex = '10000';
  toggleButton.style.fontFamily = "'Rajdhani', sans-serif";
  toggleButton.style.fontSize = '12px';
  toggleButton.style.letterSpacing = '1px';
  toggleButton.style.textTransform = 'uppercase';
  toggleButton.style.transition = 'all 0.3s ease';
  
  toggleButton.onmouseover = () => {
    toggleButton.style.backgroundColor = 'rgba(0, 195, 255, 0.2)';
    toggleButton.style.boxShadow = '0 0 10px rgba(0, 195, 255, 0.3)';
  };
  
  toggleButton.onmouseout = () => {
    toggleButton.style.backgroundColor = 'rgba(13, 37, 53, 0.9)';
    toggleButton.style.boxShadow = 'none';
  };
  
  let graphVisible = true;
  toggleButton.onclick = () => {
    graphVisible = !graphVisible;
    graphContainer.style.display = graphVisible ? 'block' : 'none';
    toggleButton.textContent = graphVisible ? 'HIDE GRAPH' : 'VIEW GRAPH';
  };
  
  const canvas = document.createElement('canvas');
  canvas.width = 320;
  canvas.height = 150;
  canvas.style.display = 'block';
  graphContainer.appendChild(canvas);

  // Add to cesiumContainer instead of body
  const cesiumContainer = document.getElementById('cesiumContainer');
  if (cesiumContainer) {
    cesiumContainer.appendChild(graphContainer);
    cesiumContainer.appendChild(toggleButton);  // Add button separately
  } else {
    console.error('Could not find cesiumContainer');
    document.body.appendChild(graphContainer);
    document.body.appendChild(toggleButton);
  }

  console.log('Graph container created:', graphContainer);

  // Update graph on clock tick
  viewer.clock.onTick.addEventListener((clock) => {
    updateAltitudeGraph(canvas, clock);
  });
}

/**
 * @brief Update altitude graph with current flight data
 * 
 * @param {*} canvas 
 * @param {*} clock 
 */
function updateAltitudeGraph(canvas, clock) {
  const ctx = canvas.getContext('2d');
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  
  const flightStats = analyzeFlightData(flightData);
  const { min, max } = flightStats.height;  // These are in kilometers
  
  // Constants for graph layout
  const padding = {
    left: 60,
    right: 10,
    top: 10,
    bottom: 30
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
  
  // Horizontal grid lines (altitude in feet)
  // Use a smaller conversion factor - 1km = 3.28084 ft (divide by 1000)
  const maxHeightFeet = max * 3.28084;
  const altitudeStep = Math.ceil(maxHeightFeet / 5); // Adjust step size accordingly
  for (let alt = 0; alt <= maxHeightFeet; alt += altitudeStep) {
    const y = padding.top + graphHeight - (alt / maxHeightFeet) * graphHeight;
    
    // Grid line
    ctx.beginPath();
    ctx.moveTo(padding.left, y);
    ctx.lineTo(canvas.width - padding.right, y);
    ctx.stroke();
    
    // Label (in feet)
    ctx.fillText(alt.toFixed(0) + 'ft', padding.left - 5, y + 4);
  }

  // Draw atmospheric layers from bottom to top
  const layers = calculateAtmosphereLayers(flightStats);
  layers.forEach((layer, i) => {
    const yBottom = padding.top + graphHeight - (layer.minHeight / max) * graphHeight;
    const yTop = padding.top + graphHeight - (layer.maxHeight / max) * graphHeight;
    const layerHeight = yBottom - yTop;
    
    ctx.fillStyle = layer.color.toCssColorString();
    ctx.fillRect(
      padding.left,
      yTop,
      graphWidth,
      layerHeight
    );

    // Add layer boundary lines
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.3)';
    ctx.beginPath();
    ctx.moveTo(padding.left, yTop);
    ctx.lineTo(padding.left + graphWidth, yTop);
    ctx.stroke();
  });

  // Plot altitude line with gradient
  const gradient = ctx.createLinearGradient(0, padding.top, 0, padding.top + graphHeight);
  gradient.addColorStop(0, 'rgba(255, 100, 100, 1)');  // Red at high altitude
  gradient.addColorStop(1, 'rgba(100, 255, 100, 1)');  // Green at low altitude

  ctx.beginPath();
  ctx.strokeStyle = gradient;
  ctx.lineWidth = 2;
  
  flightData.forEach((point, i) => {
    const heightInKm = point.height;
    const x = padding.left + (i / (flightData.length - 1)) * graphWidth;
    const y = padding.top + graphHeight - (heightInKm / max) * graphHeight;
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
    const heightInKm = flightData[currentIndex].height;
    const heightInFeet = heightInKm * 3.28084; // Correct feet conversion
    const x = padding.left + (currentIndex / (flightData.length - 1)) * graphWidth;
    const y = padding.top + graphHeight - (heightInKm / max) * graphHeight;
    
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

    // Show current altitude in feet
    ctx.fillStyle = 'white';
    ctx.font = 'bold 12px Arial';
    ctx.textAlign = 'left';
    ctx.fillText(`${heightInFeet.toFixed(0)}ft`, x + 8, y - 8);
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