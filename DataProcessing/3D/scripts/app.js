// Provide access token for Cesium Ion
Cesium.Ion.defaultAccessToken = 'your_access_token';

// Initialize the Cesium viewer with customized settings
const viewer = new Cesium.Viewer('cesiumContainer', {
  terrain: Cesium.Terrain.fromWorldTerrain(),
});    
viewer.scene.globe.enableLighting = true;

