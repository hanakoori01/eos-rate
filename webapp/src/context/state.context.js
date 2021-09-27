import React, { useEffect } from 'react'
import PropTypes from 'prop-types'

// import getProxyDataModeled from '../utils/modeled-proxy-data'

import {
  getProxies,
  getProxy,
  getUserDataModeled,
  getProducers,
  getProducer
} from './models'

const SharedStateContext = React.createContext()

const initialValue = {
  useDarkMode: false,
  user: null,
  blockProducers: { data: [], rows: 0 },
  selectedProducers: [],
  blockProducer: null,
  compareBPToolVisible: false,
  sortBlockProducersBy: null,
  proxies: { data: [], rows: 0 },
  selectedProxies: [],
  proxy: null,
  compareProxyToolVisible: false
}

const sharedStateReducer = (state, action) => {
  switch (action.type) {
    case 'ual':
      return {
        ...state,
        ual: action.ual
      }

    case 'userChange':
      return {
        ...state,
        user: action.user
      }

    case 'set': {
      return {
        ...state,
        ...action.payload
      }
    }

    case 'showMessage':
      return {
        ...state,
        message: action.payload
      }

    case 'hideMessage':
      return {
        ...state,
        message: null
      }

    case 'login':
      state.ual.showModal()

      return state

    case 'logout':
      state.ual.logout()

      return state

    case 'setProducers':
      return {
        ...state,
        blockProducers: action.blockProducers
      }

    case 'setProducer':
      return {
        ...state,
        blockProducer: action.blockProducer
      }

    case 'setSortProducersBy':
      return {
        ...state,
        sortBlockProducersBy: action.sortBy
      }

    case 'setCompareBPTool':
      return {
        ...state,
        compareBPToolVisible: action.isVisible
      }

    case 'setSelectedProducers':
      return {
        ...state,
        selectedProducers: action.selectedProducers
      }

    case 'setProxies':
      return {
        ...state,
        proxies: action.proxies
      }

    case 'setProxy':
      return {
        ...state,
        proxy: action.proxy
      }

    case 'setCompareProxyTool':
      return {
        ...state,
        compareProxyToolVisible: action.isVisible
      }

    case 'setSelectedProxies':
      return {
        ...state,
        selectedProxies: action.selectedProxies
      }

    default: {
      throw new Error(`Unsupported action type: ${action.type}`)
    }
  }
}

export const SharedStateProvider = ({ children, ual, ...props }) => {
  const [state, dispatch] = React.useReducer(sharedStateReducer, {
    ...initialValue,
    ual
  })
  const value = React.useMemo(() => [state, dispatch], [state])

  useEffect(() => {
    const load = async () => {
      if (ual.activeUser) {
        const user = await getUserDataModeled(ual)

        dispatch({ type: 'userChange', user })
      } else {
        dispatch({ type: 'userChange', user: ual.activeUser })
      }

      dispatch({ type: 'ual', ual })
    }

    load()
  }, [ual?.activeUser])

  return (
    <SharedStateContext.Provider value={value} {...props}>
      {children}
    </SharedStateContext.Provider>
  )
}

SharedStateProvider.propTypes = {
  children: PropTypes.node,
  ual: PropTypes.any
}

export const useSharedState = () => {
  const context = React.useContext(SharedStateContext)

  if (!context) {
    throw new Error(`useSharedState must be used within a SharedStateContext`)
  }

  const [state, dispatch] = context
  const setState = payload => dispatch({ type: 'set', payload })
  const showMessage = payload => dispatch({ type: 'showMessage', payload })
  const hideMessage = () => dispatch({ type: 'hideMessage' })
  const login = () => dispatch({ type: 'login' })
  const logout = () => dispatch({ type: 'logout' })
  const setUser = async () => {
    const user = await getUserDataModeled(state.ual)

    dispatch({ type: 'userChange', user })
  }
  const setSortBy = (sortBy, page) => {
    if (page === 'blockProducers') {
      dispatch({ type: 'setSortProducersBy', sortBy })
    } else {
      // dispatch({ type: 'setSortProducersBy', sortBy })
    }
  }

  // Block Producers Action
  const setProducers = async limit => {
    const blockProducers = await getProducers(limit)

    dispatch({ type: 'setProducers', blockProducers })
  }
  const setProducer = async account => {
    const blockProducer = await getProducer(account)

    dispatch({ type: 'setProducer', blockProducer })
  }
  const setCompareBPTool = isVisible => {
    dispatch({ type: 'setCompareBPTool', isVisible })
  }

  const setSelectedProducers = selectedProducers =>
    dispatch({ type: 'setSelectedProducers', selectedProducers })

  // Proxies Actions
  const setProxies = async limit => {
    const proxies = await getProxies(limit)

    dispatch({ type: 'setProxies', proxies })
  }
  const setProxy = async (data, saveDirectly = false) => {
    let proxy = data

    if (!saveDirectly) {
      proxy = await getProxy(data)
    }

    dispatch({ type: 'setProxy', proxy })
  }

  const setCompareProxyTool = isVisible =>
    dispatch({ type: 'setCompareProxyTool', isVisible })

  const setSelectedProxies = selectedProxies =>
    dispatch({ type: 'setSelectedProxies', selectedProxies })

  return [
    state,
    {
      setState,
      showMessage,
      hideMessage,
      login,
      logout,
      setUser,
      setProducers,
      setProducer,
      setCompareBPTool,
      setSelectedProducers,
      setSortBy,
      setProxies,
      setProxy,
      setCompareProxyTool,
      setSelectedProxies
    }
  ]
}