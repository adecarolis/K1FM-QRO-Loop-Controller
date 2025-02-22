const char* htmlContent = R"rawliteral(
  <!DOCTYPE html>
  <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>K1FM QRO Loop Controller</title>
      <style>
        body {
          font-family: 'Segoe UI', Arial, sans-serif;
          margin: 0;
          padding: 20px;
          background: #2e2e2e; /* Dark gray background */
          color: #e0e0e0; /* Light text for contrast */
          display: flex;
          justify-content: center;
          min-height: 100vh;
        }
        .container {
          width: 100%;
          max-width: 700px; /* Slightly wider for better spacing */
          display: flex;
          flex-direction: column;
          gap: 20px; /* Consistent spacing between sections */
        }
        h1 {
          font-size: 1.8em;
          color: #00cc00; /* Bright green for title, radio-style */
          text-align: center;
          margin: 0 0 10px 0;
          text-shadow: 0 0 5px rgba(0, 204, 0, 0.5);
        }
        .section {
          background: #3a3a3a; /* Slightly lighter dark gray */
          padding: 15px;
          border-radius: 10px;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
          border: 1px solid #555;
        }
        /* Status Section */
        .status {
          display: grid;
          grid-template-columns: 1fr 1fr; /* Two-column layout */
          gap: 10px;
          font-size: 1.1em;
          text-align: left;
        }
        .status span {
          color: #00cc00; /* Green for values */
          font-weight: bold;
        }
        .status .italic {
          font-style: italic;
          color: #ff9900; /* Orange for adjusted values */
        }
        /* Button Styles */
        .btn {
          background: #4CAF50; /* Default green */
          border: none;
          color: white;
          padding: 12px 20px;
          font-size: 1.1em;
          margin: 5px;
          border-radius: 6px;
          cursor: pointer;
          transition: background 0.2s, transform 0.1s;
          text-align: center;
        }
        .btn:hover {
          background: #66bb6a; /* Lighter green on hover */
        }
        .btn:active {
          background: #d32f2f !important; /* Red when clicked */
          transform: scale(0.95); /* Slight press effect */
        }
        .btn.selected {
          background: #e53935 !important; /* Red for selected (Auto mode) */
        }
        .btn.disabled {
          opacity: 0.5;
          pointer-events: none;
        }
        /* Step Control */
        .step-control {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(60px, 1fr)); /* Adaptive columns */
          gap: 10px;
        }
        .step-control .btn {
          width: 100%;
          background: #388e3c; /* Darker green for increase */
          padding: 8px 10px; /* Reduced padding for smaller screens */
          font-size: 1em; /* Slightly smaller font */
        }
        .step-control .btn.decrease {
          background: #c62828; /* Red for decrease */
        }
        @media (max-width: 400px) { /* Fine-tune for very narrow screens */
          .step-control .btn {
            padding: 6px 8px;
            font-size: 0.9em;
          }
        }
        /* Mode Control */
        .mode-control {
          display: flex;
          justify-content: space-between;
          gap: 10px;
        }
        .mode-control .btn {
          flex: 1;
          padding: 10px;
          font-size: 1em;
        }
        /* Memory Management */
        .memory-management {
          display: flex;
          justify-content: space-between;
          flex-wrap: wrap;
          gap: 10px;
        }
        .memory-management .btn {
          flex: 1;
          min-width: 120px;
          padding: 10px;
          font-size: 1em;
        }
        .adjust-button {
          background: #0288d1; /* Blue for adjust */
        }
        .adjust-button.selected {
          background: #e53935 !important; /* Red when active */
        }
        .sort-button {
          background: #757575; /* Gray for sort */
          min-width: 60px !important; /* Smaller sort button */
        }
        .save-button {
          background: #4CAF50; /* Green for save */
        }
        .delete-button {
          background: #d32f2f; /* Red for delete */
        }
        /* Memory Selection */
        .memory-select .band-label {
          font-weight: bold;
          margin: 15px 0 5px 0;
          color: #00cc00; /* Green for band labels */
          text-align: left;
        }
        .memory-select .out-of-band-label {
          color: #ff9900; /* Orange for out-of-band */
        }
        .memory-select .button-container {
          display: grid;
          grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
          gap: 10px;
        }
        .memory-select .btn {
          width: 100%;
          background: #555; /* Neutral gray for memory buttons */
        }
        .memory-select .btn.selected {
          background: #e53935 !important; /* Red for selected memory */
        }
      </style>
      <script>
        // [Your original JavaScript remains unchanged]
        let sortAscending = true;
        let adjustMode = false;
        let originalSteps = 0;
        let adjustedSteps = 0;
  
        function toggleRadioControl() {
          const radioControlButton = document.getElementById("radioControlButton");
          const isAuto = radioControlButton.textContent.includes("Auto");
          const newMode = isAuto ? "Manual" : "Auto";
          radioControlButton.textContent = `Radio Control [${newMode}]`;
          radioControlButton.classList.toggle("selected", !isAuto);
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              refreshStatus();
            }
          };
          xhttp.open("GET", "/rigctld_control/" + !isAuto, true);
          xhttp.send();
        }
  
        function toggleMemoryControl() {
          const memoryControlButton = document.getElementById("memoryControlButton");
          const isAuto = memoryControlButton.textContent.includes("Auto");
          const newMode = isAuto ? "Manual" : "Auto";
          memoryControlButton.textContent = `Memory [${newMode}]`;
          memoryControlButton.classList.toggle("selected", !isAuto);
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              refreshStatus();
            }
          };
          xhttp.open("GET", "/memory_auto/" + !isAuto, true);
          xhttp.send();
        }
  
        function toggleAdjust() {
          if (adjustMode) {
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
          var buttons = document.querySelectorAll(".memory-select .btn");
          buttons.forEach(button => {
            var btnIndex = parseInt(button.getAttribute("data-index"));
            if (btnIndex === index) {
              button.textContent = khz + " kHz - " + steps + " steps";
              button.classList.add("selected");
            } else {
              button.textContent = button.getAttribute("data-khz") + " kHz";
              button.classList.remove("selected");
            }
          });
          var stepElement = document.getElementById("currentSteps");
          stepElement.textContent = steps;
          stepElement.classList.add("italic");
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              refreshStatus();
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
              stepElement.classList.remove("italic");
              document.getElementById("currentFrequency").textContent = json.currentFrequency;
              document.getElementById("currentCapacity").textContent = json.capacity;
              const radioControlButton = document.getElementById("radioControlButton");
              radioControlButton.textContent = `Radio Control [${json.rigctldActive ? "Auto" : "Manual"}]`;
              radioControlButton.classList.toggle("selected", json.rigctldActive);
              const memoryControlButton = document.getElementById("memoryControlButton");
              memoryControlButton.textContent = `Memory [${json.automaticMemorySelection ? "Auto" : "Manual"}]`;
              memoryControlButton.classList.toggle("selected", json.automaticMemorySelection);
              updateMemoryHighlight(json.currentSteps);
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
          sortAscending = !sortAscending;
          loadMemories();
          updateSortButtonText();
        }
  
        function updateSortButtonText() {
          const sortButton = document.getElementById("sortButton");
          sortButton.textContent = sortAscending ? "↑" : "↓";
        }
  
        function saveMemory() {
          const currentFrequency = document.getElementById("currentFrequency").textContent;
          const currentSteps = document.getElementById("currentSteps").textContent;
          const frequency = prompt("Enter frequency in kHz:", currentFrequency);
          if (frequency === null || frequency.trim() === "") return;
          const steps = prompt("Enter steps:", currentSteps);
          if (steps === null || steps.trim() === "" || isNaN(steps)) {
            alert("Invalid steps value. Please enter a valid number.");
            return;
          }
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              loadMemories();
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
              const frequencyList = memories.map((memory, index) => `${memory.khz} kHz (Index: ${index})`).join("\n");
              const selectedIndex = prompt(`Select a memory to delete by entering its index:\n\n${frequencyList}`);
              if (selectedIndex === null || selectedIndex.trim() === "") return;
              const index = parseInt(selectedIndex);
              if (isNaN(index) || index < 0 || index >= memories.length) {
                alert("Invalid index. Please try again.");
                return;
              }
              var deleteRequest = new XMLHttpRequest();
              deleteRequest.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                  loadMemories();
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
          const bands = [
            { name: "160m", min: 1800, max: 2000 },
            { name: "80m", min: 3500, max: 4000 },
            { name: "60m", min: 5330, max: 5405 },
            { name: "40m", min: 7000, max: 7300 },
            { name: "30m", min: 10100, max: 10150 },
            { name: "20m", min: 14000, max: 14350 },
            { name: "17m", min: 18068, max: 18168 },
            { name: "15m", min: 21000, max: 21450 },
            { name: "12m", min: 24890, max: 24990 },
            { name: "10m", min: 28000, max: 29700 }
          ];
          const sortedBands = sortAscending ? bands : bands.slice().reverse();
          memories.sort((a, b) => a.khz - b.khz);
          const groupedMemories = {};
          const outOfBandMemories = [];
          memories.forEach(memory => {
            let foundBand = false;
            for (const band of bands) {
              if (memory.khz >= band.min && memory.khz <= band.max) {
                if (!groupedMemories[band.name]) groupedMemories[band.name] = [];
                groupedMemories[band.name].push(memory);
                foundBand = true;
                break;
              }
            }
            if (!foundBand) outOfBandMemories.push(memory);
          });
          sortedBands.forEach(band => {
            const bandMemories = groupedMemories[band.name];
            if (bandMemories && bandMemories.length > 0) {
              const bandLabel = document.createElement("div");
              bandLabel.className = "band-label";
              bandLabel.textContent = band.name + " (" + band.min + " - " + band.max + " kHz)";
              memoryContainer.appendChild(bandLabel);
              const buttonContainer = document.createElement("div");
              buttonContainer.className = "button-container";
              memoryContainer.appendChild(buttonContainer);
              bandMemories.forEach(memory => {
                var button = document.createElement("button");
                button.className = "btn";
                button.setAttribute("data-index", memory.index);
                button.setAttribute("data-steps", memory.steps);
                button.setAttribute("data-khz", memory.khz);
                button.onclick = function() { selectMemory(memory.index, memory.khz, memory.steps); };
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
          if (outOfBandMemories.length > 0) {
            const outOfBandLabel = document.createElement("div");
            outOfBandLabel.className = "out-of-band-label";
            outOfBandLabel.textContent = "Out of Band";
            memoryContainer.appendChild(outOfBandLabel);
            const buttonContainer = document.createElement("div");
            buttonContainer.className = "button-container";
            memoryContainer.appendChild(buttonContainer);
            outOfBandMemories.forEach(memory => {
              var button = document.createElement("button");
              button.className = "btn";
              button.setAttribute("data-index", memory.index);
              button.setAttribute("data-steps", memory.steps);
              button.setAttribute("data-khz", memory.khz);
              button.onclick = function() { selectMemory(memory.index, memory.khz, memory.steps); };
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
  
        <!-- Status Display -->
        <div class="section status">
          <div>Steps: <span id="currentSteps"></span><span id="adjustValue"></span></div>
          <div>Frequency: <span id="currentFrequency"></span> kHz</div>
          <div>Capacity: <span id="currentCapacity"></span> pF</div>
        </div>
  
        <!-- Mode Control -->
        <div class="section mode-control">
          <button id="radioControlButton" class="btn" onclick="toggleRadioControl()">Radio Control [Auto]</button>
          <button id="memoryControlButton" class="btn" onclick="toggleMemoryControl()">Memory [Auto]</button>
        </div>
  
        <!-- Manual Tuning -->
        <div class="section step-control">
          <button class="btn decrease" onclick="sendStepCommand('decrease', 100)">-100</button>
          <button class="btn decrease" onclick="sendStepCommand('decrease', 10)">-10</button>
          <button class="btn decrease" onclick="sendStepCommand('decrease', 5)">-5</button>
          <button class="btn decrease" onclick="sendStepCommand('decrease', 1)">-1</button>
          <button class="btn" onclick="sendStepCommand('increase', 1)">+1</button>
          <button class="btn" onclick="sendStepCommand('increase', 5)">+5</button>
          <button class="btn" onclick="sendStepCommand('increase', 10)">+10</button>
          <button class="btn" onclick="sendStepCommand('increase', 100)">+100</button>
        </div>
  
        <!-- Memory Management -->
        <div class="section memory-management">
          <button id="adjustButton" class="btn adjust-button" onclick="toggleAdjust()">Adjust</button>
          <button id="sortButton" class="btn sort-button" onclick="toggleSortOrder()">↑</button>
          <button class="btn save-button" onclick="saveMemory()">Save Memory</button>
          <button class="btn delete-button" onclick="deleteMemory()">Delete Memory</button>
        </div>
  
        <!-- Memory Selection -->
        <div class="section memory-select" id="memoryContainer">
          <!-- Memory buttons dynamically inserted here -->
        </div>
      </div>
    </body>
  </html>
)rawliteral";