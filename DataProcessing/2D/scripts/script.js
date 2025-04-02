// Global variable for CSV path
const DATA_PATH = "../data/Spring2025.csv";

// Define dimensions and margins
const margin = { top: 40, right: 30, bottom: 50, left: 80 }, // increased left margin for label
      width = 800 - margin.left - margin.right,
      height = 500 - margin.top - margin.bottom;

// Append SVG group element and translate it by margins
const svg = d3.select("svg")
    .append("g")
    .attr("transform", `translate(${margin.left},${margin.top})`);

// Fetch & parse flight CSV data
d3.csv(DATA_PATH, function(d, i) {
    if (i < 21) return null; // Filter first 21 data rows out

    if (!d["MTK3339 Year"] || !d["MTK3339 Month"] || !d["MTK3339 Day"] ||
        !d["MTK3339 Hour"] || !d["MTK3339 Minute"] || !d["MTK3339 Second"] ||
        !d["MTK3339 Altitude"]) {
        return null;
    }

    const year = +d["MTK3339 Year"];
    const month = +d["MTK3339 Month"] - 1;
    const day = +d["MTK3339 Day"];
    const hour = +d["MTK3339 Hour"] - 7; // UTC to GMT adjustment
    const minute = +d["MTK3339 Minute"];
    const second = +d["MTK3339 Second"];

    if (year !== 2025) {
        console.log("ERROR: Unexpected row:", d);
        return null;
    }

    // Convert altitude from millimeters to feet
    const altitude = +d["MTK3339 Altitude"] * 0.00328084;
    if (isNaN(altitude)) return null;

    return {
        date: new Date(year, month, day, hour, minute, second),
        altitude: altitude
    };
}).then(function(data) {
    console.log("SUCCESS: Data Processed.");
    console.log("Date domain:", d3.extent(data, d => d.date));

    const maxAltitude = d3.max(data, d => d.altitude);
    const minAltitude = d3.min(data, d => d.altitude);
    console.log(`Highest Altitude: ${maxAltitude} ft`);
    console.log(`Lowest Altitude: ${minAltitude} ft`);

    const cutoff = new Date(data[0].date);
    cutoff.setHours(11);
    cutoff.setMinutes(15);
    cutoff.setSeconds(0);

    // Only include data points prior to cutoff time
    data = data.filter(d => d.date <= cutoff);
    
    // Create scales
    const x = d3.scaleTime()
        .domain(d3.extent(data, d => d.date))
        .range([0, width]);

    const y = d3.scaleLinear()
        .domain([0, 100000])
        .range([height, 0]);

    // Calculate full SVG dimensions including margins
    const fullWidth = width + margin.left + margin.right;
    const fullHeight = height + margin.top + margin.bottom;

    // Set the SVG's overall dimensions (if not already done)
    d3.select("svg")
    .attr("width", fullWidth)
    .attr("height", fullHeight);

    // Append a header text element to the SVG (outside the main chart group)
    d3.select("svg")
    .append("text")
        .attr("x", fullWidth / 2)            // Center horizontally
        .attr("y", margin.top / 2)             // Place in the top margin
        .attr("text-anchor", "middle")         // Center the text
        .style("font-size", "18px")
        .style("font-weight", "bold")
        .text("Payload Altitude Over Time");

    // Add x-axis
    svg.append("g")
        .attr("transform", `translate(0,${height})`)
        .call(d3.axisBottom(x));

    // Add y-axis with grid lines and label as shown above
    svg.append("g")
        .call(d3.axisLeft(y).ticks(height / 40))
        .call(g => g.select(".domain").remove())
        .call(g => g.selectAll(".tick line").clone()
            .attr("x2", width)
            .attr("stroke-opacity", 0.1))


    // Append a vertical y-axis label
    svg.append("text")
        .attr("transform", "rotate(-90)")
        .attr("y", -margin.left)
        .attr("x", -height / 2)     
        .attr("dy", "1em")
        .attr("text-anchor", "middle")
        .text("Altitude (ft)");

    // Append an x-axis label
    svg.append("text")
    .attr("x", width / 2)
    .attr("y", height + margin.bottom - 5)
    .attr("text-anchor", "middle")
    .text("Time (MST)");


    // Create the line generator using the cleaned data
    const line = d3.line()
        .x(d => x(d.date))
        .y(d => y(d.altitude));

    // Append the path for the line chart
    svg.append("path")
        .datum(data)
        .attr("fill", "none")
        .attr("stroke", "steelblue")
        .attr("stroke-width", 2)
        .attr("d", line);
}).catch(function(error) {
    console.error("Error loading the CSV data:", error);
});
