const char* htmlContent = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>K1FM QRO Loop Controller</title>
      <style>
        body {
          font-family: Arial, sans-serif;
          text-align: center;
          margin: 0;
          padding: 0;
          display: flex;
          flex-direction: column;
          align-items: center;
        }
        .container {
          width: 90%;
          max-width: 600px;
        }
        h1 {
          margin-top: 1rem;
        }
        .section {
          background: #f8f8f8;
          padding: 1rem;
          border-radius: 8px;
          box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
          margin-bottom: 1rem;
        }
        .status div {
          font-size: 1.2em;
          margin: 5px 0;
        }
        .step-control {
          display: flex;
          flex-wrap: wrap;
          justify-content: center;
        }
        .btn {
          display: inline-block;
          background: #5B5;
          border: none;
          color: white;
          padding: 10px 15px;
          font-size: 1.2em;
          margin: 5px;
          border-radius: 5px;
          cursor: pointer;
          width: 80px;
          transition: background 0.1s;
        }
        .btn:active {
          background: #D00 !important;
        }
        .step-control .btn {
          flex: 1 1 calc(33.3% - 10px);
          max-width: 100px;
        }
        .memory-select .btn {
          display: inline-block;
          width: calc(33.3% - 10px); /* Three buttons per line */
          margin: 5px;
          box-sizing: border-box;
        }
        .step-grid {
          display: grid;
          grid-template-columns: repeat(3, 1fr);
          gap: 10px;
        }
        .italic {
          font-style: italic;
        }
        .selected {
          background: #D55 !important;
        }
        .band-label {
          font-weight: bold;
          margin-top: 10px;
          margin-bottom: 5px;
          color: #333;
          text-align: left;
          width: 100%;
        }
        .out-of-band-label {
          font-weight: bold;
          margin-top: 10px;
          margin-bottom: 5px;
          color: #666;
          text-align: left;
          width: 100%;
        }
        .btn-small {
          padding: 8px 12px; /* Uniform padding for small buttons */
          font-size: 1em; /* Uniform font size */
          margin: 5px;
          border-radius: 5px;
          cursor: pointer;
          width: auto; /* Allow buttons to size based on content */
          min-width: 80px; /* Ensure buttons have a minimum width */
        }
        .sort-button {
          background: #777;
        }
        .save-button {
          background: #55B;
        }
        .delete-button {
          background: #D55;
        }
        .adjust-button {
          background: #55D;
        }
        .disabled {
          opacity: 0.5;
          pointer-events: none;
        }
      </style>
      <script>
        let sortAscending = true; // Default sorting order: 60m to 10m
        let adjustMode = false;
        let originalSteps = 0;
        let adjustedSteps = 0;
  
        function toggleAdjust() {
          if (adjustMode) {
            // Call adjust/unset
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                adjustMode = false;
                document.getElementById("adjustButton").textContent = "Adjust";
                document.getElementById("adjustButton").classList.remove("selected");
                document.getElementById("currentSteps").textContent = this.responseText;
                document.getElementById("currentSteps").classList.remove("italic");
                document.getElementById("adjustValue").textContent = "";
                enableMemoryButtons(true);
              }
            };
            xhttp.open("GET", "/adjust/unset", true);
            xhttp.send();
          } else {
            // Call adjust/set
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
              if (this.readyState == 4 && this.status == 200) {
                adjustMode = true;
                originalSteps = parseInt(document.getElementById("currentSteps").textContent);
                adjustedSteps = parseInt(this.responseText);
                document.getElementById("adjustButton").textContent = "Unset Adjust";
                document.getElementById("adjustButton").classList.add("selected");
                document.getElementById("currentSteps").textContent = adjustedSteps;
                document.getElementById("currentSteps").classList.add("italic");
                document.getElementById("adjustValue").textContent = ` (${originalSteps - adjustedSteps})`;
                enableMemoryButtons(false);
              }
            };
            xhttp.open("GET", "/adjust/set", true);
            xhttp.send();
          }
        }
  
        function enableMemoryButtons(enable) {
          var buttons = document.querySelectorAll(".memory-select .btn");
          buttons.forEach(button => {
            if (enable) {
              button.classList.remove("disabled");
            } else {
              button.classList.add("disabled");
            }
          });
        }
  
        function sendStepCommand(command, step) {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              var json = JSON.parse(xhttp.responseText);
              var stepElement = document.getElementById("currentSteps");
              stepElement.textContent = json.currentSteps;
              stepElement.classList.add("italic");
              
              if (adjustMode) {
                document.getElementById("adjustValue").textContent = ` (${originalSteps - json.currentSteps})`;
              }
  
              updateMemoryHighlight(json.currentSteps);
            }
          };
          xhttp.open("GET", "/step/" + command + "/" + step, true);
          xhttp.send();
        }
  
        function selectMemory(index, khz, steps) {
          // Update UI immediately before sending the request
          var buttons = document.querySelectorAll(".memory-select .btn");
          buttons.forEach(button => {
            var btnIndex = parseInt(button.getAttribute("data-index"));
            if (btnIndex === index) {
              button.textContent = khz + " kHz - " + steps + " steps";
              button.classList.add("selected");
            } else {
              button.textContent = button.getAttribute("data-khz") + " kHz"; // Keep only frequency
              button.classList.remove("selected");
            }
          });
  
          // Update "Steps" display in the status window immediately in italic
          var stepElement = document.getElementById("currentSteps");
          stepElement.textContent = steps;
          stepElement.classList.add("italic");
  
          // Send request to backend
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              refreshStatus(); // Refresh after the selection to confirm state
            }
          };
          xhttp.open("GET", "/select/" + index, true);
          xhttp.send();
        }
  
        function refreshStatus() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              var json = JSON.parse(this.responseText);
              
              var stepElement = document.getElementById("currentSteps");
              stepElement.textContent = json.currentSteps;
              stepElement.classList.remove("italic"); // Definitive value
  
              document.getElementById("currentFrequency").textContent = json.currentFrequency;
              document.getElementById("currentCapacity").textContent = json.capacity;
  
              updateMemoryHighlight(json.currentSteps); // Update memory button color
            }
          };
          xhttp.open("GET", "/status", true);
          xhttp.send();
        }
  
        function updateMemoryHighlight(currentSteps) {
          var buttons = document.querySelectorAll(".memory-select .btn");
          buttons.forEach(button => {
            var steps = parseInt(button.getAttribute("data-steps"));
            if (steps === currentSteps) {
              button.classList.add("selected");
            } else {
              button.classList.remove("selected");
            }
          });
        }
  
        function loadMemories() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              var memories = JSON.parse(this.responseText);
              renderMemoryButtons(memories);
            }
          };
          xhttp.open("GET", "/memories", true);
          xhttp.send();
        }
  
        function toggleSortOrder() {
          sortAscending = !sortAscending; // Toggle sorting order
          loadMemories(); // Reload memories with the new sorting order
          updateSortButtonText(); // Update the button text
        }
  
        function updateSortButtonText() {
          const sortButton = document.getElementById("sortButton");
          sortButton.textContent = sortAscending ? "↑" : "↓"; // Use arrows to indicate sorting order
        }
  
        function saveMemory() {
          const currentFrequency = document.getElementById("currentFrequency").textContent;
          const currentSteps = document.getElementById("currentSteps").textContent;
          
          const frequency = prompt("Enter frequency in kHz:", currentFrequency);
          if (frequency === null || frequency.trim() === "") {
            return; // User canceled or entered nothing
          }
          
          const steps = prompt("Enter steps:", currentSteps);
          if (steps === null || steps.trim() === "" || isNaN(steps)) {
            alert("Invalid steps value. Please enter a valid number.");
            return;
          }
          
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              loadMemories(); // Refresh the list of memories after saving
            }
          };
          xhttp.open("GET", "/save/" + frequency + "/" + steps, true);
          xhttp.send();
        }
  
        function deleteMemory() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              var memories = JSON.parse(this.responseText);
              if (memories.length === 0) {
                alert("No memories to delete.");
                return;
              }
  
              // Create a list of frequencies for the user to choose from
              const frequencyList = memories.map((memory, index) => `${memory.khz} kHz (Index: ${index})`).join("\n");
              const selectedIndex = prompt(`Select a memory to delete by entering its index:\n\n${frequencyList}`);
              if (selectedIndex === null || selectedIndex.trim() === "") {
                return; // User canceled or entered nothing
              }
  
              const index = parseInt(selectedIndex);
              if (isNaN(index) || index < 0 || index >= memories.length) {
                alert("Invalid index. Please try again.");
                return;
              }
  
              // Send delete request
              var deleteRequest = new XMLHttpRequest();
              deleteRequest.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                  loadMemories(); // Refresh the list of memories after deletion
                }
              };
              deleteRequest.open("GET", "/delete/" + index, true);
              deleteRequest.send();
            }
          };
          xhttp.open("GET", "/memories", true);
          xhttp.send();
        }
  
        function renderMemoryButtons(memories) {
          var memoryContainer = document.getElementById("memoryContainer");
          memoryContainer.innerHTML = "";
  
          // Define the bands
          const bands = [
            { name: "160m", min: 1800, max: 2000 },
            { name: "80m", min: 3500, max: 4000 },
            { name: "60m", min: 5330, max: 5405 }, // Exception for 60 meters
            { name: "40m", min: 7000, max: 7300 },
            { name: "30m", min: 10100, max: 10150 },
            { name: "20m", min: 14000, max: 14350 },
            { name: "17m", min: 18068, max: 18168 },
            { name: "15m", min: 21000, max: 21450 },
            { name: "12m", min: 24890, max: 24990 },
            { name: "10m", min: 28000, max: 29700 }
          ];
  
          // Sort bands based on the current sorting order
          const sortedBands = sortAscending ? bands : bands.slice().reverse();
  
          // Sort memories by frequency
          memories.sort((a, b) => a.khz - b.khz);
  
          // Group memories by band
          const groupedMemories = {};
          const outOfBandMemories = [];
  
          memories.forEach(memory => {
            let foundBand = false;
            for (const band of bands) {
              if (memory.khz >= band.min && memory.khz <= band.max) {
                if (!groupedMemories[band.name]) {
                  groupedMemories[band.name] = [];
                }
                groupedMemories[band.name].push(memory);
                foundBand = true;
                break;
              }
            }
            if (!foundBand) {
              outOfBandMemories.push(memory);
            }
          });
  
          // Render buttons by band
          sortedBands.forEach(band => {
            const bandMemories = groupedMemories[band.name];
            if (bandMemories && bandMemories.length > 0) {
              // Add a label for the band
              const bandLabel = document.createElement("div");
              bandLabel.className = "band-label";
              bandLabel.textContent = band.name + " (" + band.min + " - " + band.max + " kHz)";
              memoryContainer.appendChild(bandLabel);
  
              // Create a container for the buttons in this band
              const buttonContainer = document.createElement("div");
              buttonContainer.className = "button-container";
              memoryContainer.appendChild(buttonContainer);
  
              // Add buttons for each memory in the band
              bandMemories.forEach(memory => {
                var button = document.createElement("button");
                button.className = "btn";
                button.setAttribute("data-index", memory.index); // Use memory.index instead of loop index
                button.setAttribute("data-steps", memory.steps);
                button.setAttribute("data-khz", memory.khz);
                button.onclick = function() { selectMemory(memory.index, memory.khz, memory.steps); };
  
                // Show frequency and steps only for the selected memory
                if (memory.selected) {
                  button.textContent = memory.khz + " kHz - " + memory.steps + " steps";
                  button.classList.add("selected");
                } else {
                  button.textContent = memory.khz + " kHz";
                }
  
                buttonContainer.appendChild(button);
              });
            }
          });
  
          // Render out-of-band memories
          if (outOfBandMemories.length > 0) {
            // Add a label for out-of-band memories
            const outOfBandLabel = document.createElement("div");
            outOfBandLabel.className = "out-of-band-label";
            outOfBandLabel.textContent = "Out of Band";
            memoryContainer.appendChild(outOfBandLabel);
  
            // Create a container for the out-of-band buttons
            const buttonContainer = document.createElement("div");
            buttonContainer.className = "button-container";
            memoryContainer.appendChild(buttonContainer);
  
            // Add buttons for each out-of-band memory
            outOfBandMemories.forEach(memory => {
              var button = document.createElement("button");
              button.className = "btn";
              button.setAttribute("data-index", memory.index);
              button.setAttribute("data-steps", memory.steps);
              button.setAttribute("data-khz", memory.khz);
              button.onclick = function() { selectMemory(memory.index, memory.khz, memory.steps); };
  
              // Show frequency and steps only for the selected memory
              if (memory.selected) {
                button.textContent = memory.khz + " kHz - " + memory.steps + " steps";
                button.classList.add("selected");
              } else {
                button.textContent = memory.khz + " kHz";
              }
  
              buttonContainer.appendChild(button);
            });
          }
        }
  
        setInterval(refreshStatus, 5000);
      </script>
    </head>
  
    <body onload="refreshStatus(); loadMemories(); updateSortButtonText();">
      <div class="container">
        
        <h1>K1FM QRO Loop Controller</h1>
  
        <!-- Current Status -->
        <div class="section status">
          <div>Steps: <span id="currentSteps"></span><span id="adjustValue"></span></div>
          <div>Frequency: <span id="currentFrequency"></span> kHz</div>
          <div>Capacity: <span id="currentCapacity"></span> pF</div>
        </div>
  
        <!-- Step Control -->
        <div class="section step-control">
          <div class="step-grid">
            <button class="btn" onclick="sendStepCommand('increase', 100)">+100</button>
            <button class="btn" onclick="sendStepCommand('increase', 10)">+10</button>
            <button class="btn" onclick="sendStepCommand('increase', 1)">+1</button>
            <button class="btn" onclick="sendStepCommand('decrease', 1)">-1</button>
            <button class="btn" onclick="sendStepCommand('decrease', 10)">-10</button>
            <button class="btn" onclick="sendStepCommand('decrease', 100)">-100</button>
          </div>
        </div>
  
        <!-- Adjust, Sort, Save, and Delete Buttons -->
        <div>
          <button id="adjustButton" class="btn btn-small adjust-button" onclick="toggleAdjust()">Adjust</button>
          <button id="sortButton" class="btn btn-small sort-button" onclick="toggleSortOrder()">↑</button>
          <button class="btn btn-small save-button" onclick="saveMemory()">Save Memory</button>
          <button class="btn btn-small delete-button" onclick="deleteMemory()">Delete Memory</button>
        </div>
  
        <!-- Memory Selection -->
        <div class="section memory-select" id="memoryContainer">
          <!-- Memory buttons will be dynamically inserted here -->
        </div>
  
      </div>
    </body>
  </html>
  )rawliteral";