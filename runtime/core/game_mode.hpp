#ifndef _CORE_GAME_MODE_HPP_
#define _CORE_GAME_MODE_HPP_

namespace eng
{

class CGameMode
{
public:
    virtual ~CGameMode() {}

    virtual int Init();

    virtual void Tick(float dt);

protected:
    CGameMode(){}
};

}

#endif