## 3D Data Visualization

### Overview
The 3D Data Visualization segment utilizes Cesium.js to transform raw payload data from high-altitude balloon flights into meaningful, interactive 3D visualizations. This initiative aims to display the payload's position and orientation within a 3D globe environment, and lays the groundwork for potential real-time tracking and visualization capabilities.

### Project Structure
Navigate to the `Spring-2025` repository under `/DataProcessing/3D` to access the relevant files:
- **`index.html`**: The entry point for the web application that hosts the Cesium viewer.
- **`scripts/app.js`**: Contains all logic for initializing the viewer, loading datasets, and managing user interactions.
- **`styles/style.css`**: Defines the visual styles for the application.
- **`README.md`**: Provides introductory documentation and setup instructions for new team members.

### Setup Instructions
1. **Prerequisites**:
   - Ensure you have an account on [Cesium ion](https://cesium.com/ion/) and obtain an access token to activate the viewer.
2. **Development Environment**:
   - Use Visual Studio Code with the Live Server extension for local development and testing.
   - Open `index.html` and launch the server using the "Go Live" button to start the Cesium viewer.

For additional setup instructions and project guidelines, please refer to the [Official Guide](https://docs.google.com/document/d/1dIcqozUU-LhrAeqJc7U-4poYH6kvHRLgSQxj3v1mSM8/edit?tab=t.0). The `CesiumJS Start Project` section is a good place to start.

### Resources
- **Raw APRS Data**: [Fall2024Data Folder](https://github.com/ASU-ASCEND/Fall-2024/tree/main/DataProcessing/Fall2024Data)
- **Processed Data**: [F24Processed Folder](https://github.com/ASU-ASCEND/Fall-2024/tree/main/DataProcessing/Traditional/F24Processed)
- **Data Processing Scripts**: [Scripts Folder](https://github.com/ASU-ASCEND/Fall-2024/tree/main/DataProcessing)

### Troubleshooting
- **Viewer Issues**: If the Earth does not render, check your access token in `app.js`. Ensure it is current and correctly placed.
- **Server Refresh**: If changes are not appearing, restart the Live Server to ensure updates are loaded.

### Contribution Guidelines
- **Documentation Updates**: Enhance or update this `README.md` if additional details are required or if there are changes to the project setup.
- **Data Handling**: Be cautious with personal access tokens and sensitive data. Do not push these to the repository.
