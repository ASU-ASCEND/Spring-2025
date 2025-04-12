// Global variable for CSV path
const DATA_PATH = "../data/Spring2025.csv";

// Define dimensions and margins for the chart area
const margin = { top: 40, right: 30, bottom: 50, left: 80 },
      width = 800 - margin.left - margin.right,
      height = 500 - margin.top - margin.bottom;

// Append the main chart to the SVG
const svg = d3.select("svg")
    .append("g")
    .attr("transform", `translate(${margin.left},${margin.top})`);

// Fetch & parse flight CSV data
let prev_external_temp = null;
let prev_internal_temp = null;
d3.csv(DATA_PATH, function(d, i) {
    if (i < 21) return null; // Filter first 21 data rows out

    // Determine if all data is present in a row
    if (!d["MTK3339 Year"] || !d["MTK3339 Month"] || !d["MTK3339 Day"] ||
        !d["MTK3339 Hour"] || !d["MTK3339 Minute"] || !d["MTK3339 Second"] ||
        !d["MTK3339 Altitude"]) {
        return null;
    }

    // Fetch pico data in farenheit
    const internal_temp = +d["BMP390 Temp (C)"]; // +d["TMP117 Temp (C)"];
    const external_temp = +d["TMP117_O Temp_O (C)"];

    // Fetch row MTK time data
    const year = +d["MTK3339 Year"];
    const month = +d["MTK3339 Month"] - 1;
    const day = +d["MTK3339 Day"];
    const hour = +d["MTK3339 Hour"] - 7; // UTC to GMT adjustment
    const minute = +d["MTK3339 Minute"];
    const second = +d["MTK3339 Second"];

    // Ensure anomalies in time data are discarded
    if (year !== 2025) {
        console.log("ERROR: Unexpected time:", d);
        return null;
    }

    // Convert altitude from millimeters to feet
    const altitude = +d["MTK3339 Altitude"] * 0.00328084;
    if (isNaN(altitude)) return null;

    // Detect sudden temperature changes in the ICM
    if (prev_external_temp !== null) {
        if (Math.abs(external_temp - prev_external_temp) > 5) {
            console.log("ERROR: Unexpected temperature:", d);
            return null;
        }
    }

    // Detect sudden temperature changes in the pico
    if (prev_internal_temp !== null) {
        if (Math.abs(internal_temp - prev_internal_temp) > 5) {
            console.log("ERROR: Unexpected temperature:", d);
            return null;
        }
    }

    // Track previous temperature
    prev_external_temp = external_temp;
    prev_internal_temp = internal_temp;

    return {
        date: new Date(year, month, day, hour, minute, second),
        altitude: altitude,
        internal_temp: internal_temp,
        external_temp: external_temp
    };
}).then(function(data) {
    console.log("SUCCESS: Data Processed.");
    console.log("Date domain:", d3.extent(data, d => d.date));

    const maxAltitude = d3.max(data, d => d.altitude);
    const minAltitude = d3.min(data, d => d.altitude);
    console.log(`Highest Altitude: ${maxAltitude} ft`);
    console.log(`Lowest Altitude: ${minAltitude} ft`);

    // Set a cutoff time (only include data points prior to 11:15 AM)
    console.log(d3.max(data, d => d.date));
    const cutoff = new Date(data[0].date);
    cutoff.setHours(11);
    cutoff.setMinutes(8);
    cutoff.setSeconds(0);
    data = data.filter(d => d.date <= cutoff);

    // Create scales for the chart area
    const x = d3.scaleTime()
        .domain(d3.extent(data, d => d.date))
        .range([0, width]);

    // Create altitude scale
    const y = d3.scaleLinear()
        .domain([0, 110000])
        .range([height, 0]);
    
    // Create internal_temp scale (C) 
    const yinternal_temp = d3.scaleLinear()
        .domain([-60, 50])
        .range([height, 0]);
    
    const yexternal_temp = d3.scaleLinear()
        .domain([-60, 50])
        .range([height, 0]);

    // y-axis with grid lines
    svg.append("g")
        .call(d3.axisLeft(y).ticks(height / 40))
        .call(g => g.select(".domain").remove())
        .call(g => g.selectAll(".tick line").clone()
            .attr("x2", width)
            .attr("stroke-opacity", 0.1));

    svg.append("g")
        .attr("transform", `translate(${width})`)
        .call(d3.axisRight(yinternal_temp).ticks(height / 40))
        .call(g => g.select(".domain").remove());

    // Create a line generator for internal_temp
    const internal_tempLine = d3.line()
        .x(d => x(d.date))
        .y(d => yinternal_temp(d.internal_temp));
    
    const external_tempLine = d3.line()
        .x(d => x(d.date))
        .y(d => yexternal_temp(d.external_temp));

    // Append the internal_temp line (choose a different stroke color, e.g., blue)
    svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", "blue")
        .attr("stroke-width", 2)
        .attr("d", internal_tempLine);

    svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", "orange")
        .attr("stroke-width", 2)
        .attr("d", external_tempLine);

    // Add Atmospheric Layer Backgrounds
    addAtmosphericLayering(x, y, data);

    // Add Axes and Labels
    addLabels(x, y);

    // Create graph legend
    addLegend();

    // Add a flight path
    addFlightPath(x, y, yinternal_temp, data);

}).catch(function(error) {
    console.error("Error loading the CSV data:", error);
});

/**
 * @brief Create the main flight path using the parsed data
 * 
 * @param {*} x 
 * @param {*} y 
 * @param {*} data 
 */
function addFlightPath(x, y, yinternal_temp, data) {
    const lineGenerator = d3.line()
        .x(d => x(d.date))
        .y(d => y(d.altitude));

    svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", "#FF2400")
        .attr("stroke-width", 2)
        .attr("d", lineGenerator);

    // Mark the highest point
    const maxDataPoint = data.reduce((max, d) => d.altitude > max.altitude ? d : max, data[0]);

    // Add an image marker
    svg.append("image")
        .attr("href", "../images/balloon.png")  // or use xlink:href for older D3 versions
        .attr("x", x(maxDataPoint.date) - 50 / 2)
        .attr("y", y(maxDataPoint.altitude) - 50)
        .attr("width", 50)
        .attr("height", 50);

    // Include information for the burst point
    const annotation = svg.append("text")
        .attr("x", x(maxDataPoint.date))
        .attr("y", y(maxDataPoint.altitude) - 40)
        .style("font-size", "15px");
    
    annotation.append("tspan")
        .text("Burst!")
        .attr("x", x(maxDataPoint.date) + 20)
        .attr("dy", "0em")
        .style("color", "#7C0A02")
        .style("font-weight", "bold")
        .style("fill", "#B22222");

    // Include altitude data
    annotation.append("tspan")
        .text(` • Altitude: ${maxDataPoint.altitude.toFixed(2)} ft.`)
        .style("font-size", "12px")
        .attr("x", x(maxDataPoint.date) + 20)
        .attr("dy", "1.2em");

    // Include time data
    const formatTime = d3.timeFormat("%b %d %H:%M:%S");
    annotation.append("tspan")
        .text(` • Time: ${formatTime(maxDataPoint.date)} MST`)
        .style("font-size", "12px")
        .attr("x", x(maxDataPoint.date) + 20)
        .attr("dy", "1.2em");
}

/**
 * @brief Create key labels to clarify data points
 * 
 * @param {*} x 
 * @param {*} y 
 */
function addLabels(x, y) {
    d3.select("svg")
    .append("text")
        .attr("x", margin.left + width / 2)
        .attr("y", margin.top / 2)
        .attr("text-anchor", "middle")
        .style("font-size", "18px")
        .style("font-weight", "bold")
        .text("Payload Internal and External Temperature");
  
    // Vertical y-axis label
    svg.append("text")
        .attr("transform", "rotate(-90)")
        .attr("y", -margin.left)
        .attr("x", -height / 2)
        .attr("dy", "1em")
        .attr("text-anchor", "middle")
        .text("Altitude (ft)");

    // Vertical y-axis label
    svg.append("text")
        .attr("transform", "rotate(-90)")
        .attr("x", -height / 2)              // center vertically along the chart
        .attr("y", width + margin.right + 20)  // position near the right edge
        .attr("text-anchor", "middle")
        .text("Temperature (°C)");

    // x-axis label
    svg.append("g")
        .attr("transform", `translate(0,${height})`)
        .call(d3.axisBottom(x));
        
    svg.append("text")
        .attr("x", width / 2)
        .attr("y", height + margin.bottom - 5)
        .attr("text-anchor", "middle")
        .text("Time (MST)");
}  

function altToTime(alt, dir, data){
    let res = null; 
    let i = 0; 
    for(i = 0; i < data.length; i++){
        if(data[i].altitude > alt && i+dir < data.length && i+dir > 0 && data[i+dir].altitude > data[i].altitude){
            res = data[i].date;
            break;
        }
    }

    if(dir < 0){
        for( ; i < data.length; i++){
            if(data[i].altitude < alt && i+dir < data.length && i+dir > 0 && data[i+dir].altitude > data[i].altitude){
                res = data[i].date;
                break; 
            }
        }
    }

    return res; 
}

/**
 * @brief Represent the layers of atmosphere traveled by the payload
 * 
 * @param {*} x 
 * @param {*} y 
 */
function addAtmosphericLayering(x, y, data) {
    // Stratosphere: ~97,000 ft to ~163,000 ft
    // svg.insert("rect", ":first-child")
    //     .attr("x", 0)
    //     .attr("y", y(110000))
    //     .attr("width", width)
    //     .attr("height", y(97000) - y(110000))
    //     .attr("fill", "#b5e2ff");

    // // Ozone: ~64,500 ft to ~97,000 ft
    // svg.insert("rect", ":first-child")
    //     .attr("x", 0)
    //     .attr("y", y(97000))
    //     .attr("width", width)
    //     .attr("height", y(64500) - y(97000))
    //     .attr("fill", "#73A5C6");

    let tp_start = altToTime(36214, 1, data);
    let tp_end = altToTime(56101, 1, data);
    let tp_start_down = altToTime(56101, -1, data); 
    let tp_end_down = altToTime(36214, -1, data); 
    console.log(tp_start)
    console.log(tp_start_down)
    console.log(tp_end_down)

    // Troposphere: 0 ft to ~35,000 ft
    svg.insert("rect", ":first-child")
        .attr("x", 0)
        .attr("y", 0)
        .attr("width", x(tp_start))
        .attr("height", height)
        .attr("fill", "#daf0ff");

    // tropopause 
    svg.insert("rect", ":first-child")
        .attr("x", x(tp_start))
        .attr("y", 0)
        .attr("width", x(tp_end) - x(tp_start))
        .attr("height", height)
        .attr("fill", "#c7e9ff");

    // Stratosphere: ~35,000 ft to ~64,500 ft
    svg.insert("rect", ":first-child")
        .attr("x", x(tp_end))
        .attr("y", 0)
        .attr("width", x(tp_start_down) - x(tp_end))
        .attr("height", height)
        .attr("fill", "#b5e2ff");

    console.log(x(tp_end_down) - x(tp_start_down))
    // tropopause
    svg.insert("rect", ":first-child")
        .attr("x", x(tp_start_down))
        .attr("y", 0)
        .attr("width", x(tp_end_down) - x(tp_start_down))
        .attr("height", height)
        .attr("fill", "#c7e9ff");

    // Troposphere: 0 ft to ~35,000 ft
    svg.insert("rect", ":first-child")
        .attr("x", x(tp_end_down))
        .attr("y", 0)
        .attr("width", width - x(tp_end_down))
        .attr("height", height)
        .attr("fill", "#daf0ff");
    
}

/**
 * @brief Create a legend to complement atmospheric layering
 * 
 */
function addLegend() {
    // Calculate full SVG dimensions
    const fullWidth = width + margin.left + margin.right;
    const fullHeight = height + margin.top + margin.bottom;
    
    // Increase width for legend
    const newFullWidth = fullWidth + 160;
    d3.select("svg")
        .attr("width", newFullWidth)
        .attr("height", fullHeight);

    // Define legend items for atmospheric layers and flight path.
    const legendData = [
      { label: "Layer (approx. start)", color: "white", type: "line" },
    //   { label: "Ozone (64.5–97k ft)", color: "#73A5C6", type: "box" },
      { label: "Stratosphere (~56k ft)", color: "#b5e2ff", type: "box" },
      { label: "Tropopause (~36k ft)", color: "#c7e9ff", type: "box" },
      { label: "Troposphere (0 ft)", color: "#daf0ff", type: "box" },
      { label: "Data", color: "white", type: "line" },
      { label: "Altitude (GPS)", color: "#ff2400", type: "line" },
      { label: "Internal Temperature", color: "blue", type: "line" },
      { label: "External Temperature", color: "orange", type: "line" }
    ];

    const legendItemSize = 15;
    const legendSpacing = 5;

    // Add legend to right-hand side
    const legendGroup = d3.select("svg")
      .append("g")
      .attr("class", "legend")
      .attr("transform", `translate(${margin.left + width + margin.right}, ${margin.top-31})`);

    // Create a group for each legend item.
    const legendItems = legendGroup.selectAll(".legend-item")
      .data(legendData)
      .enter()
      .append("g")
      .attr("class", "legend-item")
      .attr("transform", (d, i) => `translate(${10}, ${10 + i * (legendItemSize + legendSpacing)})`);

    // For each item, conditionally append either a rectangle (box) or a line.
    legendItems.each(function(d) {
      const g = d3.select(this);
      if (d.type === "line") {
        // Draw a horizontal line centered vertically in the legend item area.
        g.append("line")
          .attr("x1", 0)
          .attr("x2", legendItemSize)
          .attr("y1", legendItemSize / 2)
          .attr("y2", legendItemSize / 2)
          .attr("stroke", d.color)
          .attr("stroke-width", 2)
          .attr("stroke-linecap", "round");
      } else {
        // Draw a rectangle with a border.
        g.append("rect")
          .attr("width", legendItemSize)
          .attr("height", legendItemSize)
          .attr("fill", d.color)
          .attr("stroke", "black")
          .attr("stroke-width", 1);
      }
    });

    // Append text for each legend item.
    legendItems.append("text")
      .attr("x", legendItemSize + 5)
      .attr("y", legendItemSize / 2)
      .attr("dy", "0.35em")
      .style("font-size", "12px")
      .style("fill", "black")
      .text(d => d.label);
}