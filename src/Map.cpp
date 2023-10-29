

    sf::Vector2f playerPos = play.getPosition();
    std::cout << "player (" << playerPos.x << ", " << playerPos.y << ")";
    sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(*m_window));
    std::cout << " mouse (" << mousePos.x << ", " << mousePos.y << ")" << std::endl;
    sf::Vector2f midpoint((5 * playerPos.x + mousePos.x) / 6.0f, (5 * playerPos.y + mousePos.y) / 6.0f);

    sf::View view = m_window->getView();
    view.setCenter(midpoint);

    float distance = std::sqrt(std::pow(playerPos.x - mousePos.x, 2) + std::pow(playerPos.y - mousePos.y, 2));
    float zoomLevel = std::min(1.0f, 200.0f / distance);
    // view.zoom(zoomLevel);

    m_window->setView(view);

     // Get the position of the mouse relative to the center of the screen
    sf::Vector2f screenCenter(m_window->getSize().x / 2.0f, m_window->getSize().y / 2.0f);
    sf::Vector2f mouseOffset = sf::Vector2f(mousePos - screenCenter);

    // Get the position of the player relative to the center of the screen
    sf::Vector2f playerOffset = playerPos - screenCenter;

    // Calculate the angle between the two vectors
    float angle = std::atan2(mouseOffset.y - playerOffset.y, mouseOffset.x - playerOffset.x);
    angle = angle * 180.0f / M_PI; // Convert from radians to degrees

    // Set the player's rotation to the angle
    play.setRotation(angle);
