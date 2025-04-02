// Global variable for CSV path
const DATA_PATH = "../data/Spring2025.csv";
    
// Define dimensions and margins
const margin = { top: 20, right: 30, bottom: 30, left: 40 },
      width = 800 - margin.left - margin.right,
      height = 500 - margin.top - margin.bottom;

// Append SVG group element and translate it by margins
const svg = d3.select("svg")
    .append("g")
    .attr("transform", `translate(${margin.left},${margin.top})`);

// Fetch & parse flight CSV data
d3.csv(DATA_PATH, function(d, i) {
    if (i < 21) return null; // Filter first 11 data rows out

    // Check for incomplete MTK3339 fields.
    if (!d["MTK3339 Year"] || !d["MTK3339 Month"] || !d["MTK3339 Day"] ||
        !d["MTK3339 Hour"] || !d["MTK3339 Minute"] || !d["MTK3339 Second"] ||
        !d["MTK3339 Altitude"]) {
        return null;
    }

    // Parse the MTK date components
    const year = +d["MTK3339 Year"];
    const month = +d["MTK3339 Month"] - 1; 
    const day = +d["MTK3339 Day"];
    const hour = +d["MTK3339 Hour"] - 7; // UTC to GMT
    const minute = +d["MTK3339 Minute"];
    const second = +d["MTK3339 Second"];

    // Check for unexpected data points
    if (year !== 2025) {
        console.log("ERROR: Unexpected row:", d);
        return null;
    }

    // Parse the altitude value
    const altitude = +d["MTK3339 Altitude"] * 0.00328084; // In Feet
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

    // Only include data points prior to 11:30 AM
    data = data.filter(d => d.date <= cutoff);
    
    // Create scales
    const x = d3.scaleTime()
        .domain(d3.extent(data, d => d.date))
        .range([0, width]);

    const y = d3.scaleLinear()
        .domain([0, 100000])
        .range([height, 0]);

    // Add x-axis
    svg.append("g")
        .attr("transform", `translate(0,${height})`)
        .call(d3.axisBottom(x));

    // Add y-axis 
    svg.append("g")
    .call(d3.axisLeft(y)
          .tickFormat(d => d3.format(".2s")(d) + " ft."));

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