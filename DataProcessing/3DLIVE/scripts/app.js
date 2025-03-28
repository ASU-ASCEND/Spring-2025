// Provide access token for Cesium Ion
Cesium.Ion.defaultAccessToken = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJqdGkiOiJmNzRmZDQ5MS0zODY1LTRjYjEtOGI3Ny0wZGMwNWQ1MjVhOGMiLCJpZCI6MjI1Mjg3LCJpYXQiOjE3MjA0NTM1NzV9.k5jpBA4KmrErsokf_kxNKNcYbE8tNwCavyzJNRQOFeQ';

// Initialize the Cesium viewer with customized settings
const viewer = new Cesium.Viewer('cesiumContainer', {
  terrain: Cesium.Terrain.fromWorldTerrain(),
});
viewer.scene.globe.enableLighting = true;

// Global variables
let flightData = [];                   // Holds all received data points
let start = null, stop = null;         // Flight start and latest times
let sampledPositionProperty = new Cesium.SampledPositionProperty();
let payloadEntity = null;              // Main payload entity
let updateIntervalInSeconds = 30;      // Default update interval in seconds
let pollingInterval;                   // Reference for the polling interval

// Polling function to fetch new live data from the server
async function fetchNewData() {
  try {
    // Query the server for live data (ensure the fields match your server's output)
    const response = await fetch('/dataserver?fields=timestamp,latitude,longitude,height');
    const newData = await response.json();
    
    // Ensure we received the expected fields
    if (newData && !newData.missing_fields) {
      console.log("New data received:", newData);
      
      // Append the new data point to our array
      flightData.push(newData);
      
      // Convert the timestamp and position from the new data
      const time = Cesium.JulianDate.fromIso8601(newData.timestamp);
      const position = Cesium.Cartesian3.fromDegrees(
        parseFloat(newData.longitude),
        parseFloat(newData.latitude),
        parseFloat(newData.height)
      );
      
      // Add the new sample to the sampled position property
      sampledPositionProperty.addSample(time, position);
      
      // If this is the first data point, initialize the clock and timeline
      if (!start) {
        start = time;
        stop = time;
        viewer.clock.startTime = Cesium.JulianDate.clone(start);
        viewer.clock.stopTime = Cesium.JulianDate.clone(stop);
        viewer.clock.currentTime = Cesium.JulianDate.clone(start);
        viewer.timeline.zoomTo(start, stop);
      } else {
        // Update the stop time if the new data point is later than the current stop
        if (Cesium.JulianDate.greaterThan(time, stop)) {
          stop = time;
          viewer.clock.stopTime = Cesium.JulianDate.clone(stop);
          viewer.timeline.zoomTo(start, stop);
        }
      }
      
      // Create the payload entity on the first data point
      if (!payloadEntity) {
        payloadEntity = viewer.entities.add({
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
        // Start tracking the payload entity
        viewer.trackedEntity = payloadEntity;
      } else {
        // Update the payload's availability interval as new data extends the flight
        payloadEntity.availability = new Cesium.TimeIntervalCollection([ 
          new Cesium.TimeInterval({ start: start, stop: stop }) 
        ]);
      }
      
      // Add point entity for the new data sample
      const description = `
        <strong>Longitude:</strong> ${newData.longitude}</br>
        <strong>Latitude:</strong> ${newData.latitude}</br>
        <strong>Height:</strong> ${(parseFloat(newData.height) * 3.28084).toFixed(2)} ft</br>
        <strong>Time:</strong> ${newData.timestamp}</br>`;
      
      viewer.entities.add({
        name: "Flight Data Point (Update)",
        description: description,
        position: position,
        point: { pixelSize: 10, color: Cesium.Color.RED }
      });
    } else {
      console.log("New data missing expected fields or not available.");
    }
  } catch (error) {
    console.error("Error fetching updated flight data:", error);
  }
}

// Start polling for live data
pollingInterval = setInterval(fetchNewData, updateIntervalInSeconds * 1000);