"use strict";
(function () {
    var STORAGE_KEY = "dynamit-theme";

    function getStored() {
        try { return localStorage.getItem(STORAGE_KEY); }
        catch (e) { return null; }
    }

    function apply(choice) {
        document.documentElement.setAttribute("data-theme", choice || "auto");
    }

    // Apply immediately (before first paint) to prevent flash
    var stored = getStored() || "auto";
    apply(stored);

    function buildSwitcher() {
        var container = document.createElement("div");
        container.className = "theme-switcher";

        var options = [
            { value: "light", label: "\u2600" },
            { value: "dark",  label: "\uD83C\uDF19" },
            { value: "auto",  label: "\u25D0" }
        ];

        options.forEach(function (opt) {
            var btn = document.createElement("button");
            btn.textContent = opt.label;
            btn.title = opt.value.charAt(0).toUpperCase() + opt.value.slice(1);
            if (opt.value === stored) btn.classList.add("active");

            btn.addEventListener("click", function () {
                container.querySelectorAll("button").forEach(function (b) {
                    b.classList.remove("active");
                });
                btn.classList.add("active");
                apply(opt.value);
                try { localStorage.setItem(STORAGE_KEY, opt.value); }
                catch (e) { }
            });

            container.appendChild(btn);
        });

        document.body.appendChild(container);
    }

    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", buildSwitcher);
    } else {
        buildSwitcher();
    }
})();
