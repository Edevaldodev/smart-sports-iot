const API_URL = "http://localhost:8000/api/dashboard/ATLETA_07";

async function atualizarDashboard() {
    try {
        const response = await fetch(API_URL);
        if (!response.ok) return;

        const data = await response.json();

        document.getElementById('distancia').innerText = data.distancia_km;
        document.getElementById('velocidade').innerText = data.velocidade_kmh;
        document.getElementById('saltos').innerText = data.saltos;
        document.getElementById('mudancas').innerText = data.mudancas_direcao;
        
        const zonaEl = document.getElementById('zona');
        zonaEl.innerText = data.zona_esforco;

        // Alterar cor da zona de esforço
        if (data.zona_esforco === 'Alta') zonaEl.style.color = '#ef4444';
        else if (data.zona_esforco === 'Moderada') zonaEl.style.color = '#f59e0b';
        else zonaEl.style.color = '#10b981';

    } catch (err) {
        console.error("Erro ao buscar dados da API:", err);
    }
}

// Atualização automática a cada 1 segundo
setInterval(atualizarDashboard, 1000);
atualizarDashboard();