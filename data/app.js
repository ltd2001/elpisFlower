function updateStatus() {
  fetch("/status").then(res => res.json()).then(data => {
    document.getElementById("status").innerText = data.present ? "✅ 感應中" : "❌ 無卡片";
    document.getElementById("data").innerText = data.label || "--";
  });
}
setInterval(updateStatus, 5000);
