// Dynamic Card Generator - Creates animated cards with random colors
const createCard = (title, content) => {
    const colors = ['#ff6b6b', '#4ecdc4', '#45b7d1', '#96ceb4', '#feca57', '#ff9ff3'];
    const randomColor = colors[Math.floor(Math.random() * colors.length)];
    
    return `
        <div class="card" style="background: ${randomColor}; padding: 20px; margin: 10px; 
             border-radius: 12px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); 
             transform: scale(1); transition: all 0.3s ease;">
            <h3 style="margin: 0 0 10px 0; color: white;">${title}</h3>
            <p style="margin: 0; color: white; opacity: 0.9;">${content}</p>
        </div>`;
};

// Generate random cards and inject into DOM
const generateCards = () => {
    const titles = ['Innovation', 'Design', 'Technology', 'Creativity', 'Future'];
    const contents = ['Amazing ideas', 'Beautiful layouts', 'Smart solutions', 'Endless possibilities', 'Tomorrow today'];
    
    const container = document.createElement('div');
    container.innerHTML = '<h1 style="text-align: center; color: #333;">Dynamic Cards</h1>';
    
    for(let i = 0; i < 5; i++) {
        container.innerHTML += createCard(titles[i], contents[i]);
    }
    
    document.body.appendChild(container);
    
    // Add hover effects
    document.querySelectorAll('.card').forEach(card => {
        card.addEventListener('mouseenter', () => card.style.transform = 'scale(1.05)');
        card.addEventListener('mouseleave', () => card.style.transform = 'scale(1)');
    });
};

generateCards();